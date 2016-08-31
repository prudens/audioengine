#include "audio_gain_control.h"
#include <cstring>
#include <numeric>
#include <iostream>

#include "webrtc/common_audio/real_fourier.h"
#include "webrtc/common_audio/include/audio_util.h"


AudioGainControl::AudioGainControl()
{

}

AudioGainControl::~AudioGainControl()
{
    if ( m_bInit )
    {
        delete m_data;
        delete m_overLap;
        delete m_win;
        delete m_insign;
    }
}

bool AudioGainControl::Init( int32_t frame_size, SpecAnalyze analyze, int nOverLapPencent )
{
    if ( frame_size <= 0 || nOverLapPencent < 0 || nOverLapPencent >= 100 )
    {
        return false;
    }
    m_SpecAnalyze = analyze;
    m_len2 = frame_size; 
    m_len = m_len2 * 100 / (100-nOverLapPencent);
    if (m_len % 2 == 1)
    {
        m_len++;
    }
    m_len1 = m_len - m_len2; // overlap

    m_data = new float[m_len];
    memset( m_data, 0, m_len*sizeof(float) );

    m_overLap = new float[m_len1];
    memset( m_overLap, 0, m_len1*sizeof( float ) );

    m_win = new float[m_len];
    int16_t* win = new int16_t[m_len];
    WebRtcSpl_GetHanningWindow( win, m_len/2 );//Q14
    for ( int i = 0; i < m_len/2; i++ )
    {
        m_win[i] = (float)win[i] / (1<<14);
    }
    for ( int i = 0; i < m_len / 2; i++ )
    {
        m_win[m_len / 2 + i] = m_win[m_len/2 - i - 1];
    }
    delete win;
    win = nullptr;

    m_fftorder = sizeof( int32_t ) * 8 - WebRtcSpl_NormW32( m_len );
    m_nFFT = static_cast<int32_t>(std::pow( 2, m_fftorder ));


    m_fft = webrtc::RealFourier::Create( m_fftorder );
    m_spec.resize( m_nFFT );
    m_amplitude.resize( m_nFFT );
    m_angle.resize( m_nFFT );
    m_insign = new float[m_nFFT];
    memset( m_insign, 0, m_nFFT * sizeof(float) );
    return true;
}

void AudioGainControl::ScaleVoice( int16_t* data, int16_t size )
{
    memcpy( m_data, m_data + m_len - m_len1, m_len1*sizeof( float ) );
    webrtc::S16ToFloat( data, m_len2, m_data + m_len1 );
    for ( int i = 0; i < m_len; i++ )
    {
        m_insign[i] = m_win[i] * m_data[i];
    }

    m_fft->Forward( m_insign, &m_spec[0] );

   // m_fft->Inverse( m_spec, m_insign ); // test

    for ( int i = 0; i < m_nFFT; i++ )
    {
        m_amplitude[i] = std::pow( std::abs( m_spec[i] ), 2 );
        m_angle[i] = std::arg( m_spec[i] );
    }

    bool bInverse = false;
    if ( m_SpecAnalyze )
    {
        bInverse = m_SpecAnalyze( m_spec, m_amplitude, m_angle );
    }

    if (!bInverse)
    {
        memcpy( m_overLap, m_insign + m_len - m_len1, m_len1 );
        return;
    }

    std::valarray<float> amp_r = sqrt( m_amplitude )* cos( m_amplitude );
    std::valarray<float> amp_i = sqrt( m_amplitude )* sin( m_amplitude );

    for ( int i = 0; i < m_nFFT; i++ )
    {
        m_spec[i] = Complex( amp_r[i], amp_i[i] );
    }

    m_fft->Inverse( &m_spec[0], m_insign );
    for ( int i = 0; i < m_len1; i++ )
    {
        m_insign[i]  = m_overLap[i] + m_insign[i]; //
        m_overLap[i] = m_insign[m_len - m_len1 + i];
    }

    webrtc::FloatToS16( m_insign, m_len1, data ); 
}

int32_t AudioGainControl::nextpow2( int32_t num )
{
    int32_t n = sizeof( int32_t ) * 8 - 1 - WebRtcSpl_NormW32( num );
    return static_cast<int32_t>(pow( 2, n ));
}


