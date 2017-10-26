#include "audio_noise_suppression.h"
#include <math.h>
#include <string.h>
#include <algorithm>
#include "webrtc/common_audio/include/audio_util.h"
#include "kwindows.h"
#include <functional>
#include <valarray>
#include <boost/math/special_functions/bessel.hpp>
#include "audio_parse_param.h"

namespace snail{
    namespace audio{
        SignalStat g_sig_stat;


        AudioNoiseSuppression::AudioNoiseSuppression()
        {
            m_file = fopen( "d:/dump.txt", "w+" );
            AudioParseParameter::GetInstance().RegisterNotify( this );
            ParseParamNotify( "" );
            for ( int i = 0; i < 24; i++ )
            {
                m_audio_type_list[i] = -1;
            }
        }

        AudioNoiseSuppression::~AudioNoiseSuppression()
        {
            if ( m_file )
            {
                fclose( m_file );
            }
            for ( auto v : audio_cache_free )
            {
                delete v;
            }
            if(m_energy)
                delete m_energy;
        }

        void AudioNoiseSuppression::Init( int samplerate, int nChannel )
        {
            m_fft.Init( samplerate / 100, std::bind( &AudioNoiseSuppression::ProcessSpec, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
            m_frame = m_fft.FFTSize();
            m_noise.resize( m_frame );
            m_energy = new float[m_frame];
            m_Xk_prev = m_noise;
            m_recSamplerate = samplerate;
            m_recChannel = nChannel;
            m_agc.Init( samplerate, nChannel );
            m_find_peaks.SetMinPeakDistance( 5 );
            m_find_peaks.SetMinPeakHeight( 0.0f );
        }

        AudioNoiseSuppression::VoiceFrame* AudioNoiseSuppression::Proceess( int16_t* data, size_t nSamples, bool silent )
        {
            if ( data )
            {
                m_bsilent = silent;
                m_fft.Process( data, nSamples );
            }

            return ProceessNS( data, nSamples, 0 );
        }

        void AudioNoiseSuppression::Reset()
        {
            m_future_list.clear();
            m_init_num = 0;
        }


        bool AudioNoiseSuppression::ProcessMMSE( std::valarray<float>& sig )
        {
            if ( !m_bEnableMMSE )
            {
                return false;
            }
            size_t size = sig.size();
            for ( size_t i = 0; i < size; i++ )
            {
                if ( sig[i] == 0 )
                {
                    sig[i] += 0.000001;
                }
            }
            m_init_num++;
            if ( m_init_num < 6 )
            {
                m_noise += sig;
                return false;
            }
            if ( m_init_num == 6 )
            {
                m_noise += sig;
                m_noise /= 6;
                m_noise *= m_noise;
            }

            static float c = 0.8862f; //sqrt(PI)/2
            static float aa = 0.98f;
            static float ksi_min = pow( 10, -2.5 );// note that in Chap. 7, ref.[17], ksi_min( dB ) = -15 dB is recommended
            std::valarray<float> amplitide = pow( sig, 2.0f );
            std::valarray<float> gammak = amplitide / m_noise;//posteriori SNR
            gammak = gammak.apply( [] ( float v ) { return std::min( v, 40.0f ); } );

            std::valarray<float> ksi = m_Xk_prev / m_noise * aa + ( 1 - aa ) * gammak.apply( [] ( float v ) { return std::max( v - 1.0f, 0.0f ); } );
            ksi = ksi.apply( [] ( float v ) { return std::max( v, ksi_min ); } );
            std::valarray<float> log_sigma_k = gammak*ksi / ( ksi + 1.0f ) - log( ksi + 1.0f );
            auto vad_decision = log_sigma_k.sum() / sig.size();
            if ( vad_decision < 0.2f )
            {
                m_noise = m_noise * 0.98f + amplitide * ( 0.02f );
            }
            std::valarray<float> vk = ksi*gammak / ( ksi + 1.0f );
            std::valarray<float> j0 = vk.apply( [] ( float v ) { return (float)boost::math::cyl_bessel_i( 0.0f, v / 2 ); } );
            std::valarray<float> j1 = vk.apply( [] ( float v ) { return  (float)boost::math::cyl_bessel_i( 1.0f, v / 2 ); } );
            std::valarray<float> C = exp( vk * -0.5f );

            std::valarray<float> A = sqrt( vk )  * c * C / gammak;
            std::valarray<float> B = ( vk + 1.f )*j0 + vk*j1;
            std::valarray<float> hw = A*B;
            sig *= hw; //最终结果。
            m_Xk_prev = sig * sig;
            return true;
        }


        void AudioNoiseSuppression::ParseParamNotify( const std::string& Param )
        {
            int32_t value = 0;
            if ( AudioParseParameter::GetInstance().GetValue( "ans", 'm', value ) )
            {
                m_bEnableMMSE = value != 0;
            }

            if (AudioParseParameter::GetInstance().GetValue("ans",'e',value))
            {
                m_bEnable = value != 0;
            }
            if ( AudioParseParameter::GetInstance().GetValue( "ans", 'r', value ) )
            {
                m_disableForRecordingMsg = value != 0;
            }
            if ( AudioParseParameter::GetInstance().GetValue( "gcr", 'b', value, 0, 2 ) )
                g_sig_stat.LowerSourceTTL = value;
        }

        bool AudioNoiseSuppression::ProcessSpec( std::valarray< std::complex<float> >&frec, std::valarray<float>& amplitide, std::valarray<float>& angle )
        {
            if ( !m_bEnable || m_disableForRecordingMsg )
            {
                m_noise_type = m_bsilent ? Silent : Speech;
                return false;
            }

            bool ret = ProcessMMSE( amplitide );// 先用MMSE算法降噪

            if ( m_bsilent )
            {      
                m_noise_type = Silent;
                AnalyzePeaks( amplitide );
            }
            else
            {
                std::nth_element( &amplitide[0], &amplitide[0] + amplitide.size() * 2/3, &amplitide[0] + amplitide.size() );
                g_sig_stat.silent_levels.push_back( amplitide[amplitide.size()/2] );
                if ( g_sig_stat.silent_levels.size() > 400 )
                {
                    g_sig_stat.silent_levels.pop_front();
                }
                float sum = 0.0f;
                for ( auto f : g_sig_stat.silent_levels )
                {
                    sum += f;
                }
                g_sig_stat.min_height = sum / 400;
                m_noise_type = AnalyzePeaks( amplitide );
                printf( "%.3f ", g_sig_stat.min_height );
            }

            m_find_peaks.SetMinPeakHeight( g_sig_stat.min_height/* * g_sig_stat.coef*/ );

            return ret;
        }

        AudioNoiseSuppression::VoiceFrame* AudioNoiseSuppression::ProceessNS( int16_t* data, size_t size, int level )
        {
            if ( !data )
            {
                // 清空列表
                if ( !audio_cache.empty() )
                {
                    auto p = audio_cache.front();
                    audio_cache.pop_front();
                    audio_cache_free.push_back( p );
                    return p;
                }
                else
                {
                    return nullptr;
                }
            }


            int noise_type = m_noise_type;



            m_future_list.push_back( noise_type );
            VoiceFrame* pFrame = nullptr;
            if ( !audio_cache_free.empty() )
            {
                pFrame = audio_cache_free.front();
                audio_cache_free.pop_front();
            }
            else
            {
                pFrame = new VoiceFrame();
            }
            memcpy( pFrame->data, data, size * 2 );
            pFrame->size = size;
            pFrame->level = level;
            pFrame->silent = m_bsilent;
            audio_cache.push_back( pFrame );
            if ( m_future_list.size() < 4 )
                return nullptr;


            auto p = audio_cache.front();
            //process begin
            memmove( m_audio_type_list, m_audio_type_list + 1, sizeof( m_audio_type_list ) - sizeof( int ) );
            m_audio_type_list[23] = noise_type;
            bool bZero = true;
            int nSpeech2 = 0;
            int nNoise2 = 0;
            for ( int i = 10; i < 20; i++ )
            {
                if ( m_audio_type_list[i] == Speech )
                {
                    nSpeech2++;
                }
                else if ( m_audio_type_list[i] == Noise )
                {
                    nNoise2++;
                }
            }



            int nSpeech3 = 0;
            int nNoise3 = 0;
            for ( int i = 20; i < 24; i++ )
            {
                if ( m_audio_type_list[i] == Speech )
                {
                    nSpeech3++;
                }
                else if ( m_audio_type_list[i] == Noise )
                {
                    nNoise3++;
                }
            }


            int nSpeech1 = 0;
            int nNoise1 = 0;
            for ( int i = 0; i < 10; i++ )
            {
                if ( m_audio_type_list[i] == Speech )
                {
                    nSpeech1++;
                }
                else if ( m_audio_type_list[i] == Noise )
                {
                    nNoise1++;
                }
            }

            if ( nSpeech1 > 6 || nSpeech2 > 4 ||  nSpeech3 > 2 )
            {
                bZero = false;
            }

            if ( bZero )
            {
                p->level = 120;
                p->silent = true;
                memset( p->data, 0, p->size * 2 );
            }
            else
            {
                if (!m_file)
                {
                    if ( !p->silent )
                        m_agc.Process( p->data, p->size );
                }

            }


            //proceess end
            audio_cache.pop_front();
            m_future_list.pop_front();

            audio_cache_free.push_back( p );

            return p;
        }

        int AudioNoiseSuppression::AnalyzePeaks( std::valarray<float>& amplitide )
        {
            FindPeaks::result_type pks;
            int noise_type = Speech;
            float std = 0.0f;
            if (!m_bsilent)
            {
                pks = m_find_peaks.Process( &amplitide[0], amplitide.size() );

                float m = 0.0f;
                size_t idx = 0;
                for ( size_t i = 0; i < pks.size(); i++ )
                {
                    if ( pks[i].first > m )
                    {
                        m = pks[i].first;
                        idx = pks[i].second;
                    }
                }
                m_noise_coef.max_freq = m;

               std = Std( pks );

                m_noise_coef.std = std;

                size_t size = pks.size();
                if ( size == 0 )
                {
                    noise_type = Consonant;
                }

                if ( size == 1 )
                {
                    float v = pks[0].first;
                    if ( v < g_sig_stat.min_height * g_sig_stat.coef )
                    {
                        noise_type = Consonant;
                    }
                    else
                    {
                        noise_type = Noise;
                    }
                }
                if ( size >= 2 )
                {
                    if ( pks[0].second > 64 && std > 5.12f / pks[0].second )
                    {
                        noise_type = Noise;
                    }
                    else if ( idx > 64 && std > 0.2f )
                    {
                        noise_type = Noise;
                    }
                    else if ( std < 0.7f )
                    {
                        // 还要判断静音和大部分的能量集中区，否则太容易有杂音进去了。
                        noise_type = Speech;
                    }
                    else
                    {
                        noise_type = Noise;
                    }
                }
                if ( noise_type == Speech )
                {
                    g_sig_stat.speech_levels.push_back( pks.size() );
                    if ( g_sig_stat.speech_levels.size() > 200 )
                    {
                        g_sig_stat.speech_levels.pop_front();
                    }
                    size_t sum = 0;
                    for ( auto v : g_sig_stat.speech_levels )
                    {
                        sum += v;
                    }
                    sum /= 200;
                    if ( sum >= 5 && noise_type != Consonant)
                    {
                        g_sig_stat.speech_diff_level++;
                    }
                    else
                    {
                        g_sig_stat.speech_diff_level--;
                    }

                    if ( g_sig_stat.speech_diff_level > 60 )
                    {
                        g_sig_stat.coef *= 1.1f;
                        g_sig_stat.speech_diff_level = 0;
                        printf( "%.3f ", g_sig_stat.coef );
                    }
                    else if ( g_sig_stat.speech_diff_level < -50 )
                    {
                        g_sig_stat.coef *= 0.9f;
                        g_sig_stat.speech_diff_level = 0;
                        printf( "-%.3f ", g_sig_stat.coef );
                    }
                }

            } 
            //debug  


           if ( m_file )
           {
               static int count = 0;
               if ( m_bsilent )
               {
                   fprintf( m_file, "[%.3d]%s \n", count, "Silent" );
                   count++;
               }
               else
               {
                   fprintf( m_file, "[%.3d]%s ", count, noise_type == Speech ? "Speech" : noise_type == Consonant ? "Consonant" : "Noise" );
                   for ( size_t i = 0; i < pks.size(); i++ )
                   {
                       fprintf( m_file, "%.4d %.3f  ", pks[i].second * 8000 / 257, ( pks[i].first ) );
                   }
                   count++;
                   fprintf( m_file, "    =%.3f\n", std );
                   //                 if (pks.size()>0)
                   //                 {
                   //                     fprintf( m_file, "%d,%.3f\n", idx * 8000 / 257, m );
                   //                 }
                   //                 else
                   //                 {
                   //                     fprintf( m_file, "%d,%.3f\n", 0, 0.0f );
                   //                 }

               }
           }
           //debug
            return noise_type;
        }

        float AudioNoiseSuppression::Std( const std::vector<std::pair<float, size_t>>& pks )
        {
            if (pks.size() < 2)
            {
                return 0.0f;
            }
            float sum_sq = 0.0f;
            float sum = 0.0f;
            for ( size_t i = 0; i < pks.size(); i++ )
            {
                sum += pks[i].first;
            }
            auto mean = sum / pks.size();
            for ( size_t i = 0; i < pks.size(); i++ )
            {
                sum_sq += ( pks[i].first - mean )*( pks[i].first - mean );
            }
            sum_sq /= pks.size() - 1;
            sum_sq = sqrt( sum_sq );
            sum_sq /= sum;
            return sum_sq;
        }

}}

