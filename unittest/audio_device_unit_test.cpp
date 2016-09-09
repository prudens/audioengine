#include <iostream>
#include <list>
#include <mutex>
#include <algorithm>
#include <cassert>
#include <complex>
#include <thread>
#include <conio.h>
#include <algorithm>

#include "base/fft.h"
#include "base/audio_util.h"
#include "device/include/audio_device.h"
#include "effect/3d/include/mit_hrtf_lib.h"
#include "io/wav_file.h"
#include "base/circular_buffer.hpp"
#include "effect/3d/include/mixer3d.h"
#include "audio_effect.h"
#include "base/time_cvt.hpp"
#include "base/min_max_heap.hpp"
#include <numeric>
#include "audio_noise_suppression.h"
#include "webrtc/common_audio/signal_processing/include/spl_inl.h"
#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"

//#include "dispatch_example.cpp"

// Example: main calls myfunc
extern int test_vcl( int argc, char* argv[] );
#ifdef _DEBUG
#pragma comment(lib,"../build/winx/Debug/audio_device.lib")
#pragma comment(lib,"../build/winx/Debug/audio_effect.lib")
#pragma comment(lib,"../build/winx/Debug/audio_io.lib")
#pragma comment(lib,"../build/winx/Debug/audio_base.lib")
#pragma comment(lib,"../build/winx/Debug/audio_processing.lib")
#pragma comment(lib,"../build/winx/Debug/libmpg123.lib")
#pragma comment(lib,"../build/winx/Debug/libmp3lame.lib")
#pragma comment(lib,"../build/winx/Debug/aac.lib")
#else
#pragma comment(lib,"../build/winx/Release/audio_device.lib")
#pragma comment(lib,"../build/winx/Release/audio_effect.lib")
#pragma comment(lib,"../build/winx/Release/audio_io.lib")
#pragma comment(lib,"../build/winx/Release/audio_base.lib")
#pragma comment(lib,"../build/winx/Release/audio_processing.lib")
#pragma comment(lib,"../build/winx/Release/libmpg123.lib")
#pragma comment(lib,"../build/winx/Release/libmp3lame.lib")
#pragma comment(lib,"../build/winx/Release/aac.lib")
#endif
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "msdmo")
#pragma comment(lib, "dmoguids")
#pragma comment(lib, "wmcodecdspuuid")
#pragma comment(lib,"winmm.lib")
using namespace std;
typedef lock_guard<mutex> lockguard;

using  Complex = std::complex < float > ;


//typedef std::complex<float> complex;
// 卷积公式
//y(n)=sum(x(k)h(n-k)), 其中 n-k>0,0<n<N,0<k<M
// 参见《信号与系统》第50页
template <class T>
void conv( T *x, T* h, T *y, size_t xn, size_t hn, size_t yn )
{
    memset( y, 0, sizeof( T ) * yn);
    for ( size_t n = 0; n < yn; n++ )
        for ( size_t j = 0; j < xn; j++ )
            if ( n -j >= 0 && n - j < hn )
                y[n] += x[j] * h[n - j];
}

#define TEST_CONV
void convolution( const float*input, Complex*irc, float *output, int nFFT, int nFil, int nSig )
{
#ifdef TEST_CONV
    for (int i = 0; i < nFFT; i++)
    {
        if ( i < nSig )
        {
            output[i] = input[i];
        }
        else
        {
            output[i] = 0;
        }
    }
    return;
#endif

    Complex*inc, *outc;
    inc = new Complex[nFFT];
    for ( int i = 0; i < nFFT; i++ )
    {
        if ( i < nSig )
        {
            inc[i] = Complex( (float)input[i] );
        }
        else
        {
            inc[i] = Complex( 0 );
        }
    }

    CFFT::Forward( inc, nFFT );
    outc = new Complex[nFFT];
    for ( int i = 0; i < nFFT; i++ )
    {
        outc[i] = inc[i] * irc[i];
    }

    CFFT::Inverse( outc, nFFT );

    for ( int i = 0; i < nFFT; i++ )
    {
        output[i] = (float)( outc[i].real() );
    }
    delete[] inc;
    delete[] outc;
}



class CAudioBufferProc : public  AudioBufferProc
{
    bool m_processhrtf;
    list<char*> m_list;
    mutex   m_lock;
    Mixer3D m_Mixer3D;
    AudioEffect* pEffect;
public:
    CAudioBufferProc(bool processhrtf) :m_processhrtf(processhrtf), m_Mixer3D(48000)
    {
         int nAzimuth = 90;
         int nElevation = 0;
         m_Mixer3D.updateAngles( nAzimuth, nElevation );
         pEffect = new AudioEffect;
         pEffect->Init( 48000, 2, 48000, 2 );
    }

    virtual void RecordingDataIsAvailable( const void*data, size_t samples )
    {
        lockguard lg( m_lock );
        pEffect->ProcessCaptureStream( (int16_t*)data, samples );

        if (!pEffect->HasVoice())
        {
            //printf( "cur frame is silent\n" );
        }
        if (m_processhrtf)
        {
            m_Mixer3D.AddProcessData( (int16_t*)data, samples / 2 );
            for ( ;; )
            {
                char* pData = new char[samples];
                size_t readsample = m_Mixer3D.GetProcessData( (int16_t*)pData, samples / 2 );
                if (readsample == 0)
                {
                    delete[] pData;
                    return;
                }
               
                m_list.push_back( (char*)pData );
            }
        }
        else
        {
            char* pData = new char[samples];
            if (!pEffect->HasVoice())
            {
                memset( pData, 0, samples );
            }
            else
            {
                memcpy( pData, data, samples );
            }
            m_list.push_back( (char*)pData );
        }
    }

    virtual size_t NeedMorePlayoutData( void* data, size_t samples )
    {

        lockguard lg( m_lock );
        if (m_list.size()<50)
        {
            return samples;
        }
        
         char* p = m_list.front();
         m_list.pop_front();
       memcpy( data, p, samples );
         delete p;
         return samples;
    }

    virtual void ErrorOccurred(AudioError aeCode) {}
    void UpdateAngles( int nAzimuth, int nElevation )
    {
        m_Mixer3D.updateAngles( nAzimuth, nElevation );
    }
};


void test_conv()
{
    // test conv time remains
    int16_t x[3] = { 1, 2, 3 };
    int16_t h[5] = { 1, 3, 5, 7, 9 };
    int16_t y[10] = { 0 };
    conv( x, h, y, 3, 5, 10 );
    int16_t res[10] = {1,5,14,26,38,39,27,0,0,0};
    assert( !memcmp( y, res, sizeof( int16_t ) * 10 ) );
    conv( h, x, y, 5, 3, 10 );
    assert( !memcmp( y, res, sizeof( int16_t ) * 10 ) );
    for ( int i = 0; i < 10; i++ )
    {
        //std::cout << y[i] << "\t";
    }
    std::cout << endl;
    Complex data[] = { 1, 2, 3, 4 };
    bool b = CFFT::Forward( data, 4 );
    assert( b );
    b = CFFT::Inverse( data, 4 );
    assert( b );
    for ( int i = 0; i < 4; i++ )
    {
       // cout << data[i].re() << ",";
        assert( data[i].real() == i+1 );
    }

    //test convolution

}

void test_windows_core_audio()
{
    AudioDevice* pWinDevice = AudioDevice::Create();

    pWinDevice->Initialize();
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();

    CAudioBufferProc cb(false);
    pWinDevice->SetAudioBufferCallback( &cb );
    pWinDevice->StartPlayout();
    pWinDevice->StartRecording();

    system( "pause" );

    pWinDevice->StopRecording();
    pWinDevice->StopPlayout();
    system( "pause" );
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();
    pWinDevice->StartPlayout();
    pWinDevice->StartRecording();
    system( "pause" );
    pWinDevice->Terminate();
    pWinDevice->Release();
}

void test_hrtf( int nAzimuth, int nElevation, const char* inputfile, const char*outputfile )
{

    WavReader reader( inputfile );
    int len = reader.num_samples();

    const int nFFT = 4096;
    float pLeft[nFFT] = {0};
    float pRight[nFFT] = { 0 };
    int nFil = mit_hrtf_get( &nAzimuth, &nElevation, reader.sample_rate(), 0, pLeft, pRight );
    if ( nFil == 0 ) return;
    Complex flt[nFFT] = { 0 };
    for ( int i = 0; i < nFil;i++ )
    {
        flt[i] = Complex(pLeft[i]);

    }
    CFFT::Forward( flt, nFFT );

    WavWriter writer( outputfile, reader.sample_rate(), 1 );

    int16_t pSrc[nFFT*2];
    reader.ReadSamples( nFFT*reader.num_channels(), pSrc );
    reader.ReadSamples( nFFT*reader.num_channels(), pSrc );
    reader.ReadSamples( nFFT*reader.num_channels(), pSrc );
    int16_t pMono[nFFT];
    DownmixInterleavedToMono(pSrc,nFFT,reader.num_channels(),pMono);
    writer.WriteSamples( pMono, nFFT );
    float pData[nFFT];
    S16ToFloat(pMono,nFFT,pData);
    float output[nFFT];
    convolution(pData,flt,output,nFFT,nFil,nFFT);
    FloatToS16( output, nFFT,pSrc );

    writer.WriteSamples( pSrc, nFFT );
}

void test_real_time_3d()
{
    int nAzimuth = 0; int nElevation = 0;
    AudioDevice* pWinDevice = AudioDevice::Create();
    pWinDevice->Initialize();
    pWinDevice->SetRecordingFormat( 48000, 2 );
    pWinDevice->SetPlayoutFormat( 48000, 2 );
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();

    CAudioBufferProc cb( true);
    pWinDevice->SetAudioBufferCallback( &cb );
    pWinDevice->StartPlayout();
    pWinDevice->StartRecording();
    char ch = '\n';
    printf( "\n" );
    do
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        if ( _kbhit() )
        {
            
            ch = _getch();
            switch ( ch )
            {
            case 'a':
                nAzimuth -= 15;
                break;
            case 'd':
                nAzimuth += 15;
                break;
            case 'w':
                nElevation += 10;
                break;
            case 's':
                nElevation -= 10;
                break;
            default:
                printf( "invalid argment:%c",ch );
            }
#define fuck
            nAzimuth = std::min fuck( std::max fuck ( nAzimuth, -180 ), 180 );
            nElevation = std::min fuck( std::max fuck ( nElevation, -40 ), 90 );
            cb.UpdateAngles( nAzimuth, nElevation );
            
            printf( "nAzimuth:%d,nElevation:%d\r", nAzimuth, nElevation );
        }


    } while ( ( ch != 'Q' ) && ( ch != 'q' ) );

    pWinDevice->StopRecording();
    pWinDevice->StopPlayout();
    pWinDevice->Terminate();
    pWinDevice->Release();
}


void test_mit_hrtf_get()
{
    int nAzimuth = 0;
    int nElevation = 0;
    int nFil = mit_hrtf_availability( nAzimuth, nElevation, 48000, 0 );
    int16_t *irL_ = nullptr;
    int16_t *irR_ = nullptr;
    float* fl = nullptr;
    float* fr = nullptr;
    if ( nFil )
    {
        irL_ = new int16_t[nFil];
        irR_ = new int16_t[nFil];
        fl = new float[nFil];
        fr = new float[nFil];
        nFil = mit_hrtf_get( &nAzimuth, &nElevation, 48000, 0, irL_, irR_ );
        nFil = mit_hrtf_get( &nAzimuth, &nElevation, 48000, 0, fl, fr );
        short* flt = new short[nFil];
        short* frt = new short[nFil];
        FloatToS16( fl, nFil, flt );
        FloatToS16( fr, nFil, frt );

        for ( int i = 0; i < nFil; i++ )
        {
            //cout << flt[i] << "---" << fl[i] << '\n';
            //cout << frt[i] << "---" << fr[i] << '\n';
            assert( flt[i] == irL_[i] && frt[i] == irR_[i] );
        }
        delete[] irR_;
        delete[] irL_;
        delete[] fl;
        delete[] fr;
        delete[] flt;
        delete[] frt;
    }
    else
    {
        return;
    }
}

void test_circular_buffer()
{
    const int capacity = 10;
    CircularAudioBuffer buffer(capacity);
    assert(0 == buffer.readSizeRemaining());
    assert( 10 == buffer.writeSizeRemaining() );
    int16_t data[4] = { 1, 2, 3, 4 };
    assert(4 == buffer.write(data,4));
    assert( 6 == buffer.writeSizeRemaining() );
    assert( 4 == buffer.readSizeRemaining() );
    int16_t ReadData[4] = { 0 };
    assert( 4 == buffer.read( ReadData, 4 ) );
    assert( !memcmp( ReadData, data, 4 * sizeof( int16_t ) ) );
    assert( 0 == buffer.read( ReadData, 3 ) );
    int16_t data10[10] = {1,2,3,4,5,6,7,8,9,10};
    assert( 10 == buffer.write( data10, 10 ) );
    assert( 0 == buffer.write( data, 4 ) );
}
#include "io/include/audioreader.h"
#include "io/include/audiowriter.h"

class Mp3ReadProc : public  AudioBufferProc
{
    AudioReader* pMp3Reader;
    AudioWriter* pMp3Writer;
    int64_t   m_ts = 0;
public:
    Mp3ReadProc()
    {
        pMp3Reader = AudioReader::Create("E:/CloudMusic/Mariage.mp3",AFT_MP3);
       // pMp3Reader->SetSpeed( 1 );
        pMp3Writer = AudioWriter::Create( "D:/myvoice.aac",44100,2, AFT_AAC );
        std::cout << timestamp() << std::endl;
        m_ts = timestamp();
    }
    ~Mp3ReadProc()
    {
        auto ts = timestamp();
        std::cout << "\n record time: " << ts - m_ts << " sample number： " << pMp3Writer->NumSamples() / pMp3Writer->SampleRate()/pMp3Writer->NumChannels() << std::endl;
        pMp3Reader->Destroy();
        pMp3Writer->Destroy();
    }
    virtual void RecordingDataIsAvailable( const void*data, size_t size_in_byte )
    {
        pMp3Writer->WriteSamples( (int16_t*)data, size_in_byte/2 );
    };
    virtual size_t NeedMorePlayoutData( void*data, size_t size_in_byte )
    {
        int len = pMp3Reader->ReadSamples( size_in_byte / 2, (int16_t*)data );
        pMp3Writer->WriteSamples( (int16_t*)data, size_in_byte / 2 );
        if (len == 0)
        {
            pMp3Reader->SeekTime( 0 );
        }
        return size_in_byte;
    }
};



void test_play_mp3()
{
    AudioDevice* pWinDevice = AudioDevice::Create();
    pWinDevice->Initialize();
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();

    Mp3ReadProc cb;
    pWinDevice->SetAudioBufferCallback( &cb );
    pWinDevice->StartPlayout();
  //  pWinDevice->StartRecording();
    system( "pause" );
    pWinDevice->StopPlayout();
    pWinDevice->StopRecording();
    pWinDevice->Terminate();
    pWinDevice->Release();
}


void test_audio_processing()
{
    WavReader reader_rec( "D:/rec-97.wav" );
    int samplerate = reader_rec.sample_rate();
    int channel = reader_rec.num_channels();

    WavWriter writer( "d:/log/test-pro.wav", samplerate, channel );
    AudioEffect ae;
    ae.Init( samplerate, channel, samplerate, channel );
    int frames = samplerate / 100 * channel;
    int16_t* buf = new int16_t[frames];
    for ( ;; )
    {
        if ( frames != reader_rec.ReadSamples( frames, buf ) )
             break;
        ae.ProcessCaptureStream( buf, frames*2 );

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
const int BAND_NUM =16;
//int band[BAND_NUM+1] = { 80, 500, 1100, 1750, 2500,3500,4500,6000, 8000};
const float Threshold[BAND_NUM] = {1.f,0.7f,0.65f,0.65f,0.65f,0.6f,0.6f,0.6f};
const int CoefVar[BAND_NUM] = { 4, 1, 1, 1, 1, 1, 1, 1 };
int Band[BAND_NUM + 1] = { 80, 500, 1000, 1500,2000,2500,3000,3500,4000,4500,5000,5500,6000,6500,7000,7500,8000 };
struct bandinfo
{
    float v;
    int idx;
    int start;
    int end;
};

float GetStd(float*arr,int size)
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
    auto fft = webrtc::RealFourier::Create(8);
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
        int end   = static_cast<int>( Band[i+1] * bandunit );
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


    int start = static_cast<int>(cbi[0].start * bandunit);
    int end = static_cast<int>( cbi[0].end * bandunit);
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
    if ( sum  < 1)
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
        samplerate = reader_rec.sample_rate();
        channel = reader_rec.num_channels();
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
            if (pFrame)
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
        samplerate = reader_rec.sample_rate();
        channel = reader_rec.num_channels();
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

            for ( int i = min(history_list.size(), query_list.size()); i > 0; i-- )
            {
                if ( *itbefore == 0  )
                {
                    h_noise--;
                }
                else if ( *itbefore == 3  )
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

            if ( h_noise == 0)
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
            else if ( h_noise  < 0 )
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
            if (history_list.size()>10)
            {
                history_list.pop_front();
            }
        }
    }
}

#include "codec/aac/libAACenc/include/aacenc_lib.h"
void test_aac_enc()
{
    WavReader reader( "E:\\CloudMusic\\Mariage.wav" );
    int channel = reader.num_channels();
    int samplerate = reader.sample_rate();

    auto aacfile = AudioWriter::Create( "D:/Mariage.aac", samplerate, channel, AFT_AAC );
    int frame = 882;
    int16_t*buf = new int16_t[frame];
    for ( ;; )
    {
        auto len = reader.ReadSamples( 882, buf );
        if (len>0)
        {
            aacfile->WriteSamples( buf, len );
        }
        else
        {
            break;
        }
    }

    aacfile->Destroy();
    delete[] buf;
}

#include "codec/aac/libAACdec/include/aacdecoder_lib.h"
void test_aac_dec()
{
    WavWriter* writer = nullptr;// ( "D:/myvoice.wav" );
    FILE* file = fopen( "D:/myvoice.aac","rb" );
    HANDLE_AACDECODER aacDecoderInfo = aacDecoder_Open( TT_MP4_ADTS, 1 );
    
    AAC_DECODER_ERROR err;
    err = aacDecoder_SetParam( aacDecoderInfo, AAC_DRC_REFERENCE_LEVEL, 1 );
    uint8_t* buf = new uint8_t[2048];
    UINT bufsize = 2048;
    INT_PCM* outbuf = new INT_PCM[4096];
    CStreamInfo* pStreamInfo = nullptr;
    UINT bytevalid=0;
    for ( ;; )
    {
        if (bytevalid == 0)
        {
            bufsize = fread( buf, 1, 2048, file );
            if ( bufsize == 0 )
            {
                break;
            }
            bytevalid = bufsize;
        }
        UINT startpos = bufsize - bytevalid;
        bufsize = bytevalid;
        UCHAR*p = buf + startpos;
        err = aacDecoder_Fill( aacDecoderInfo, &p, &bufsize, &bytevalid );
        if ( err != AAC_DEC_OK )
        {
            printf( "aacDecoder_Fill() failed: 0x%x", err );
            return;
        }
        if ( bytevalid != 0 )
        {
            printf("bytevalid = %d\n",bytevalid);
        }
        for ( ;; )
        {
            err = aacDecoder_DecodeFrame( aacDecoderInfo, outbuf, 4096, 0 );
            if ( err == AAC_DEC_NOT_ENOUGH_BITS )
            {
                break;
            }
            else if ( err != AAC_DEC_OK )
            {
                break;
            }
            if ( !pStreamInfo )
            {
                pStreamInfo = aacDecoder_GetStreamInfo( aacDecoderInfo );
                writer = new WavWriter( "D:/myvoice.wav", pStreamInfo->sampleRate, pStreamInfo->numChannels );
            }
            writer->WriteSamples( outbuf, pStreamInfo->frameSize*pStreamInfo->numChannels );
        }

    }
    aacDecoder_Close( aacDecoderInfo );
    if(writer)delete writer;
    delete[] buf;
    delete[] outbuf;
}
struct AACDecHeader 
{
    uint16_t Syncword;//12
    uint8_t version;//1
    uint8_t Layer;//2
    uint8_t ProtAbsent;//1
    uint8_t Profile;//2
    uint8_t sampleindex;//4
    uint8_t Private_Stream;//1
    uint8_t ChannelMode;//3;
    uint8_t Originality;//1
    uint8_t Home;//1
    uint8_t cr_Stream;//1
    uint8_t cr_Start;//1
    uint16_t frame_length;//13
    uint16_t buf_fullness;//11
    uint8_t num_frame;//2
    uint16_t CRC;//16
};
void test_aac_pasre_head()
{
    //‭111111111111 0 00 1 01 0111 0 010 0 0 0 0 0000010111001 00010101000 00‬
    FILE* file = fopen( "D:/myvoice.aac", "rb" );
    uint8_t header[7] = { 0 };
    AACDecHeader aacHeader;
    int index = 0;
    int seek = 0;
    for ( ;; )
    {
        if ( 7 != fread( header, 1, 7, file ) )
        {
            break;
        }
        uint8_t*p = header;
        aacHeader.Syncword = *p;
        p++;
        aacHeader.Syncword <<= 4;
        aacHeader.Syncword += (*p >> 4);
        aacHeader.version = (*p >> 3) & 0x1;
        aacHeader.Layer = (*p >> 1) & 0x3;
        aacHeader.ProtAbsent = *p & 0x1;
        p++;
        aacHeader.Profile = *p >> 6;//2
        aacHeader.sampleindex = ( *p >> 2 ) & 0x0f;//4
        aacHeader.Private_Stream = ( *p >> 1 ) & 0x1;
        aacHeader.ChannelMode = ( *p ) & 0x1;
        aacHeader.ChannelMode <<= 2;
        p++;
        aacHeader.ChannelMode += ( *p >> 6 ) & 0x3;//2
        aacHeader.Originality = ( *p >> 5 ) & 0x1;//1
        aacHeader.Home = ( *p >> 4 ) & 0x1;//1
        aacHeader.cr_Stream = ( *p >> 3 ) & 0x1;//1
        aacHeader.cr_Start = ( *p >> 2 ) & 0x1;//1
        aacHeader.frame_length = ( *p ) & 0x3;//2
        ++p;
        aacHeader.frame_length <<= 8;
        aacHeader.frame_length |= *p;
        aacHeader.frame_length <<= 3;
        ++p;
        aacHeader.frame_length |= *p >> 5;//3

        aacHeader.buf_fullness = *p & 0x1f;//5
        ++p;
        aacHeader.buf_fullness <<= 6;
        aacHeader.buf_fullness |= *p >> 2;//6
        aacHeader.num_frame = *p & 0x3;
        index++;
        printf( "[%d]frame_len = %d\t\t",index, aacHeader.frame_length );

        seek += aacHeader.frame_length;
        fseek(file,seek,SEEK_SET);
    }

    printf( "\n\n\n\n\n\n\nframe number=%d,sample_num=%d,time=%d", index, 4096 * ( index ), 4096 * ( index )/44100/2 );
    fclose( file );
}

#include "io/include/audioreader.h"
void test_aac_dec_file()
{
    AudioReader* pReader = AudioReader::Create( "D:/myvoice.aac", AFT_AAC );
    printf( "channel:%d",pReader->NumChannels() );
    printf( "samplerate:%d", pReader->SampleRate() );
    printf( "number sample:%d", pReader->NumSamples() );
    WavWriter writer( "D:/myvoice.wav", pReader->SampleRate(), pReader->NumChannels() );
    int16_t buf[4096];
    for ( int i = 0; i < 100;i++ )
    {
        int len = pReader->ReadSamples( 3000, buf );
        if ( len == 0 )
        {
            break;
        }
        writer.WriteSamples( buf, len );
    }
    pReader->SeekSamples( pReader->NumSamples() /2);
    for ( ;; )
    {
        int len = pReader->ReadSamples( 3000, buf );
        if (len == 0 )
        {
            break;
        }
        writer.WriteSamples( buf, len );
    }

    pReader->Destroy();
}
#include <filesystem>

void run_mp32wav( const char* filename )
{
    using file_path = std::tr2::sys::path;

    AudioReader *mp3reader = AudioReader::Create( filename, AFT_MP3 );
    if (mp3reader == nullptr)
    {
        return;
    }
    file_path p1 = filename;
    p1.replace_extension( ".wav" );

    WavWriter writer( p1.string().c_str(), mp3reader->SampleRate(), mp3reader->NumChannels() );
    int16_t buf[4096];
    for ( ;; )
    {
        int len = mp3reader->ReadSamples( 4096, buf );
        if (len == 0)
        {
            break;
        }
        writer.WriteSamples( buf, len );
    }
    mp3reader->Destroy();
}


int walk( int x, int y )
{
    if ( x < 1 || y < 1 )
    {
        return 0;
    }
    if ( x == 1 || y == 1 )
    {
        return 1;
    }
    return walk( x - 1, y ) + walk( x, y - 1 );
}
#include "base/asyntask.h"
void test_async_task()
{
    AsynTask asyn_task;
    for ( auto i : {5,2,3,3,1} )
    asyn_task.PostTask( i, false, [] ( uint32_t id ) { printf( "%d  ", id ); } );

    system( "pause" );
}

#include "webrtc/common_audio/real_fourier.h"
#include "base/time_cvt.hpp"
void test_fft()
{
    const int order = 512;
    using namespace webrtc;
    auto pfft = RealFourier::Create( lround(log10(order)) );
    float arr[order];
    for ( int i = 0; i < order; i++ )
    {
        arr[i] = (float)i;
    }
    std::complex<float> caar[order];

    Timer t;
    for ( int i = 0; i < 2000; i++ )
    {
        pfft->Forward( arr, caar );
        pfft->Inverse( caar, arr );
    }
    cout << "Ooura Implemented fft processed time: "<<t.elapsed() <<"ms"<< endl;

    for ( int i = 0; i < order; i++ )
    {
        caar[i] = (float)i;
    }
    t.reset();
    for ( int i = 0; i < 2000; i++ )
    {
        CFFT::Forward( caar, order );
        CFFT::Inverse( caar, order );
    }

    cout <<"LIBROW implemented fft processed time :"<< t.elapsed() << "ms"<<endl;
    t.reset();
}


#include "audio_gain_control.h"

void test_voice_scale()
{
    WavReader reader_rec( "D:/log/test-16000.wav" );
    int samplerate = reader_rec.sample_rate();
    int channel = reader_rec.num_channels();
    WavWriter writer( "D:/log/test1.wav", samplerate, channel );

    AudioGainControl agc;

    const int size = 160;
    agc.Init( size, [&size] ( std::valarray< std::complex<float> >&frec, std::valarray<float>& amplitide, std::valarray<float>& angle ) {
        for ( int i = 0; i < size/8; i++ )
        {
            amplitide[i] *= 2;
        }
        return false; } );
    int16_t pPCMData[size] = { 0 };
    for ( ;; )
    {
        if ( 0 == reader_rec.ReadSamples( size, pPCMData ) )
        {
            break;
        }
        agc.ScaleVoice( pPCMData, size );
        writer.WriteSamples( pPCMData, size );
    }
}


void test_array();
int main( int argc, char** argv )
{
    test_audio_processing();
   // test_async_task();
   // test_windows_core_audio();
   // test_conv();
   // test_hrtf(45,0,"C:/Users/zhangnaigan/Desktop/3D_test_Audio/es01.wav","D:/pro-48000-1.wav");
   // test_real_time_3d();
   // test_mit_hrtf_get();
    //test_circular_buffer();
   // test_play_mp3();
   // test_vcl( argc, argv );
   // test_audio_ns();
   // test_audio_processing();


  //  test_aac_enc();
//    test_aac_dec();
//    test_aac_pasre_head();
 //   test_aac_dec_file();
//    run_mp32wav( "E:/CloudMusic/凡人歌.mp3" );

    //test_array();
   // test_fft();
    
   // test_voice_scale();
    return 0;
    const int nFFT = 256;
    float arr[nFFT] = { 2, 2, 3, 4 };
    int16_t a[nFFT] = { 0 };
    Complex c[nFFT] = { 0 };
    float m_amplitude[nFFT] = { 0 };
    float m_angle[nFFT] = { 0 };

    rtc::scoped_ptr< webrtc::RealFourier > fft = webrtc::RealFourier::Create( 4 );
    for ( int k = 0; k < 5; k++ )
    {
        for ( int i = 0; i < nFFT; i++ )
        {
            a[i] = i;
        }
        fft->Forward( arr, c );
        for ( int i = 0; i < 4; i++ )
        {
            m_amplitude[i] = std::pow( std::abs( c[i] ), 2 );
            m_angle[i] = std::arg( c[i] );
        }
        for ( int i = 0; i < 50; i++ )
        {
            m_amplitude[i] *= 4;
        }
        float amp_r[nFFT];
        float amp_i[nFFT];
        for ( int i = 0; i < 4; i++ )
        {
            auto amp = sqrt( m_amplitude[i] );
            amp_r[i] = amp * cosf( m_angle[i] );
            amp_i[i] = amp * sinf( m_angle[i] );
        }

        for ( int i = 0; i < nFFT; i++ )
        { 
            c[i] = Complex( amp_r[i], amp_i[i] );
        }
        fft->Inverse( c, arr );
        for ( int i = 0; i < 10;i++ )
        {
            std::cout << a[i] << "  ";
        }
        std::cout << endl;
    }

    system( "pause" );
    return 0;

}