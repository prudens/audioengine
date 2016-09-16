#include "header.h"
void test_audio_effect()
{
    WavReader reader_rec( "D:/rec-97.wav" );
    int samplerate = reader_rec.SampleRate();
    int channel = reader_rec.NumChannels();

    WavWriter writer( "d:/log/test-pro.wav", samplerate, channel );
    AudioEffect ae;
    ae.Init( samplerate, channel, samplerate, channel );
    int frames = samplerate / 100 * channel;
    int16_t* buf = new int16_t[frames];
    for ( ;; )
    {
        if ( frames != reader_rec.ReadSamples( frames, buf ) )
            break;
        ae.ProcessCaptureStream( buf, frames * 2 );

        //while (ae.GetRecordingData(buf,frames*2,false))
        {
            writer.WriteSamples( buf, frames );
        }
    }
    while ( ae.GetRecordingData( buf, frames * 2, true ) )
    {
        writer.WriteSamples( buf, frames );
    }
    delete[] buf;
}


//  频率带划分
//        80 Hz - 250 Hz
//        250 Hz - 500 Hz
//        500 Hz - 1000 Hz
//        1000 Hz - 2000 Hz
//        2000 Hz - 3000 Hz
//        3000 Hz - 4000 Hz
//        4000 Hz - 8000 Hz
//        8000 Hz - 24000 Hz
const int frame = 160;
const int BAND_NUM = 16;
//int band[BAND_NUM+1] = { 80, 500, 1100, 1750, 2500,3500,4500,6000, 8000};
const float Threshold[BAND_NUM] = { 1.f, 0.7f, 0.65f, 0.65f, 0.65f, 0.6f, 0.6f, 0.6f };
const int CoefVar[BAND_NUM] = { 4, 1, 1, 1, 1, 1, 1, 1 };
int Band[BAND_NUM + 1] = { 80, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000 };
struct bandinfo
{
    float v;
    int idx;
    int start;
    int end;
};

float GetStd( float*arr, int size )
{
    float mean = 0;
    for ( int i = 0; i < size; i++ )
    {
        mean += arr[i];
    }
    mean /= size;
    float std = 0;
    for ( int i = 0; i < size; i++ )
    {
        std += ( arr[i] - mean ) * ( arr[i] - mean );
    }

    std /= size;
    std = sqrt( std );
    std /= mean;
    return std;
}
enum NoiseType
{
    Speech,       // 正常语音
    Consonant,    // 辅音,声音比较轻，不排除是音节的前后所带的辅音，如果前后是噪音，那么他也是噪音，类似填充作用
    FullHighFreq, // 无低频区域，这个无人声，不排除是噪音和辅音 
    Noise,        // 噪音，纯噪音，但是不排除是正常声音的一部分
    Silent,       // 静音，能量太小。
};

NoiseType AnalyzeNoise( int16_t* pPCMData, int /*frame*/ )
{
    int samplerate = 16000;
    static int k = 0;
    k++;
    auto fft = webrtc::RealFourier::Create( 8 );
    NoiseType type;
    const float bandunit = frame / (float)samplerate;
    Complex pData[frame];// = new complex[frame];
    float proc_buf[frame];
    float bands[41];
    float energy[frame] = { 0 };
    float std[10] = { 0 };

    for ( int i = 0; i < frame; i++ )
    {
        proc_buf[i] = S16ToFloat( pPCMData[i] )/**hw[i]*/;
    }

    fft->Forward( proc_buf, pData );

    float sum = 0;
    for ( int i = 0; i < BAND_NUM; i++ )
    {
        bands[i] = 0;
        int start = static_cast<int>( Band[i] * bandunit );
        int end = static_cast<int>( Band[i + 1] * bandunit );
        for ( int j = start; j < end; j++ )
        {
            energy[j] = ( pData[j].real() * pData[j].real() + pData[j].imag() * pData[j].imag() );
            bands[i] += energy[j];
        }
        bands[i] /= ( end - start );
    }
    bandinfo bi[BAND_NUM];
    for ( int i = 0; i < BAND_NUM; i++ )
    {
        bi[i].idx = i;
        bi[i].v = bands[i];
        bi[i].start = Band[i];
        bi[i].end = Band[i + 1];
    }

    sort( bi, bi + BAND_NUM, [] ( const bandinfo& l, const bandinfo& r ) { return l.v > r.v; } );
    bandinfo cbi[BAND_NUM];
    int index = 0;
    // 合并
    for ( int i = 0; i < BAND_NUM; i++ )
    {
        if ( i == BAND_NUM - 1 )
        {
            cbi[index].v = bi[i].v;
            cbi[index].start = bi[i].start;
            cbi[index].end = bi[i].end;
            index++;
            break;
        }

        int sub = bi[i].idx - bi[i + 1].idx;
        if ( sub == -1 )
        {
            cbi[index].v = ( bi[i].v + bi[i + 1].v ) / 2;
            cbi[index].start = bi[i].start;
            cbi[index].end = bi[i + 1].end;
            i++;

        }
        else  if ( sub == 1 )
        {
            cbi[index].v = ( bi[i].v + bi[i + 1].v ) / 2;
            cbi[index].start = bi[i + 1].start;
            cbi[index].end = bi[i].end;
            i++;
        }
        else
        {
            cbi[index].v = bi[i].v;
            cbi[index].start = bi[i].start;
            cbi[index].end = bi[i].end;
        }
        index++;
    }
    sum = 0;
    for ( int i = 0; i < index; i++ )
    {
        sum += bi[i].v;
    }

    float speech_proc = bands[0] / sum;
    float scale = cbi[0].v / sum;


    int start = static_cast<int>( cbi[0].start * bandunit );
    int end = static_cast<int>( cbi[0].end * bandunit );
    int len = end - start + 1;
    len /= 5;
    for ( int i = 0; i < 5; i++ )
    {
        std[i] = 0;
        for ( int j = 0; j < len; j++ )
        {
            std[i] += energy[start + i * len + j];
        }
    }

    sort( std, std + 5, std::greater<float>() );
    float s = 0;
    for ( int i = 0; i < 5; i++ )
    {
        s += std[i];
    }
    float focus = ( std[0] ) / s;
    if ( sum < 1 )
    {
        type = Silent;
        printf( "[%d-%d]静音数据\n", ( k - 1 ) * frame * 1000 / samplerate, k * frame * 1000 / samplerate );
    }
    else if ( bi[0].idx == 0 )
    {
        type = Speech;
        printf( "[%d-%d]正常说话声\n", ( k - 1 ) * frame * 1000 / samplerate, k * frame * 1000 / samplerate );
    }
    else if ( cbi[0].start == 80 && focus < 0.9 )
    {
        type = Speech;
        printf( "[%d-%d]正常说话声\n", ( k - 1 ) * frame * 1000 / samplerate, k * frame * 1000 / samplerate );
    }
    else if ( 0.8 < scale )
    {
        type = Noise;
        printf( "[%d-%d]噪音 %d   %.3f focus=%f    \n", ( k - 1 ) * frame * 1000 / samplerate, k * frame * 1000 / samplerate, bi[0].idx, scale, focus );
        for ( int i = 0; i < 5; i++ )
        {
            printf( "%.6f  ", std[i] );
        }
        printf( "\n" );
    }
    else if ( 0.3 < scale  && focus > 0.6 )
    {
        type = Noise;
        printf( "[%d-%d]噪音 %d   %.3f focus=%f   \n", ( k - 1 ) * frame * 1000 / samplerate, k * frame * 1000 / samplerate, bi[0].idx, scale, focus );
        for ( int i = 0; i < 5; i++ )
        {
            printf( "%.6f  ", std[i] );
        }
        printf( "\n" );
    }
    else if ( speech_proc < 0.01 && 0.4 < scale && focus > 0.4 )
    {
        type = FullHighFreq;
        printf( "[%d-%d]人声频率太少%.6f    \n", ( k - 1 ) * frame * 1000 / samplerate, k * frame * 1000 / samplerate, speech_proc );
    }
    else
    {
        type = Consonant;
        printf( "[%d-%d]可能是辅音\n", ( k - 1 ) * frame * 1000 / samplerate, k * frame * 1000 / samplerate );
    }

    return type;
}
void test_voice_scale( int16_t* data, int16_t size );
void test_audio_ns()
{
    AudioNoiseSuppression ans;
    int samplerate;
    int channel;
    std::list<int> level_list;
    int16_t pPCMData[frame] = { 0 };
    {
        WavReader reader_rec( "D:/log/123.wav" );
        samplerate = reader_rec.SampleRate();
        channel = reader_rec.NumChannels();
        WavWriter writer( "D:/log/test1.wav", samplerate, channel );
        ans.Init( samplerate, channel );
        for ( ;; )
        {
            if ( 0 == reader_rec.ReadSamples( frame, pPCMData ) )
            {
                break;
            }
            int type = ans.Analyze( pPCMData, frame, false );
            level_list.push_back( type );

            auto pFrame = ans.Proceess( pPCMData, frame, 0, false, type );
            if ( pFrame )
            {
                writer.WriteSamples( pFrame->data, pFrame->size );
            }
        }
    }

    for ( const auto & v : level_list )
    {
        printf( "%d  ", v );
    }

    return;
    {

        WavReader reader_rec( "D:/log/test-16000.wav" );
        samplerate = reader_rec.SampleRate();
        channel = reader_rec.NumChannels();
        WavWriter writer( "D:/log/test1.wav", samplerate, channel );
        for ( const auto & v : level_list )
        {
            printf( "%d  ", v );
        }
        printf( "\n" );
        // 这里开始分析数据
        int last_nosie = 0;
        std::list<int> query_list;
        std::list<int> history_list;
        for ( int i = 0; i < 6; i++ )
        {
            history_list.push_back( 0 );
        }
        for ( auto v : level_list )
        {

            query_list.push_back( v );
            if ( query_list.size() < 6 )
            {
                continue;
            }

            //process begin
            int h_noise = -1;
            auto itbefore = history_list.begin();
            auto itafter = query_list.begin();

            for ( int i = min( history_list.size(), query_list.size() ); i > 0; i-- )
            {
                if ( *itbefore == 0 )
                {
                    h_noise--;
                }
                else if ( *itbefore == 3 )
                {
                    h_noise++;
                }

                if ( *itafter == 0 )
                {
                    h_noise--;
                }
                else if ( *itafter == 3 )
                {
                    h_noise++;
                }

                if ( h_noise >= 3 )
                {
                    break;
                }
                if ( h_noise <= -3 )
                {
                    break;
                }
                itbefore++;
                itafter++;
            }

            if ( h_noise == 0 )
            {
                h_noise = last_nosie;
            }


            if ( h_noise > 0 )
            {
                reader_rec.ReadSamples( frame, pPCMData );
                memset( pPCMData, 0, frame * 2 );

                writer.WriteSamples( pPCMData, frame );
                printf( "%d  ", h_noise );
            }
            else if ( h_noise < 0 )
            {
                reader_rec.ReadSamples( frame, pPCMData );
                writer.WriteSamples( pPCMData, frame );
                printf( "%d  ", h_noise );
            }
            last_nosie = h_noise;
            //proceess end
            int cur = query_list.front();
            query_list.pop_front();
            history_list.push_front( cur );
            if ( history_list.size() > 10 )
            {
                history_list.pop_front();
            }
        }
    }
}

#include "processing/src/audio_mixer/include/audio_mixer.h"
class AudioStream :public MixerParticipant
{
public:
    AudioStream( const char* file )
    {
        reader = new WavReader( file );
        channel = reader->NumChannels();
        samplerate = reader->SampleRate();
    }
    virtual bool GetAudioFrame( webrtc::AudioFrame* audioFrame )
    {
        int nSamples = samplerate / 100 * channel;
        if ( nSamples != reader->ReadSamples( nSamples, audioFrame->data_ ) )
             return false;
        audioFrame->num_channels_ = channel;
        audioFrame->sample_rate_hz_ = samplerate;
        audioFrame->vad_activity_ = AudioFrame::kVadActive;
        audioFrame->samples_per_channel_ = 480;
        return true;
    }
    ~AudioStream()
    {
        delete reader;
    }
private:
    WavReader*reader;
    int samplerate;
    int channel;
};
void test_audio_mixer()
{
    WavWriter output( "D:/es-output.wav",48000,2 );
    AudioStream es01("C:/Users/zhangnaigan/Desktop/3D_test_Audio/es01.wav");
    AudioStream es02( "C:/Users/zhangnaigan/Desktop/3D_test_Audio/es02.wav" );
    AudioStream es03( "C:/Users/zhangnaigan/Desktop/3D_test_Audio/es03.wav" );
    AudioStream es04( "C:/Users/zhangnaigan/Desktop/3D_test_Audio/sc01.wav" );
    AudioStream es05( "C:/Users/zhangnaigan/Desktop/3D_test_Audio/sc02.wav" );
    AudioStream es06( "C:/Users/zhangnaigan/Desktop/3D_test_Audio/sc03.wav" );
    AudioMixer* mixer = AudioMixer::Create( 48000, 2 );
    mixer->LimitParticipantCount( 5 );
    mixer->AddParticipant( &es01 );
    mixer->AddParticipant( &es02 );
    mixer->AddParticipant( &es03 );
    mixer->AddParticipant( &es04 );
    mixer->AddParticipant( &es05 );
    mixer->AddParticipant( &es06 );
    AudioFrame af;
    af.samples_per_channel_ = 480;
    af.sample_rate_hz_ = 48000;
    af.num_channels_ = 2;
    while ( mixer->GetMixerAudio( &af ) )
    {
        output.WriteSamples( af.data_, af.num_channels_*af.samples_per_channel_ );
    }
}

void test_audio_processing()
{
    test_audio_mixer();
}