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

#define  complex std::complex<float>


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
void convolution( const float*input, complex*irc, float *output, int nFFT, int nFil, int nSig )
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

    complex*inc, *outc;
    inc = new complex[nFFT];
    for ( int i = 0; i < nFFT; i++ )
    {
        if ( i < nSig )
        {
            inc[i] = complex( (float)input[i] );
        }
        else
        {
            inc[i] = complex( 0 );
        }
    }

    CFFT::Forward( inc, nFFT );
    outc = new complex[nFFT];
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
    complex data[] = { 1, 2, 3, 4 };
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
    complex flt[nFFT] = { 0 };
    for ( int i = 0; i < nFil;i++ )
    {
        flt[i] = complex(pLeft[i]);

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
        pMp3Reader->SetSpeed( 2 );
        pMp3Writer = AudioWriter::Create( "D:/myvoice.mp3",44100,2, AFT_MP3 );
        std::cout << timestamp() << std::endl;
        m_ts = timestamp();
    }
    ~Mp3ReadProc()
    {
        pMp3Reader->Destroy();
        pMp3Writer->Destroy();
        auto ts = timestamp();
        std::cout << ts - m_ts << std::endl;
    }
    virtual void RecordingDataIsAvailable( const void*data, size_t size_in_byte )
    {
        pMp3Writer->WriteSamples( (int16_t*)data, size_in_byte/2 );
    };
    virtual size_t NeedMorePlayoutData( void*data, size_t size_in_byte )
    {
//        float buf[441 * 4];
//        pMp3Reader->ReadSamples( size_in_byte / 2, buf );
//        FloatToS16(buf,size_in_byte/2,(int16_t*)data);
        int len = pMp3Reader->ReadSamples( size_in_byte / 2, (int16_t*)data );
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
  //  pWinDevice->StartPlayout();
    pWinDevice->StartRecording();
    system( "pause" );
    pWinDevice->StopPlayout();
    pWinDevice->StopRecording();
    pWinDevice->Terminate();
    pWinDevice->Release();
}


void test_audio_processing()
{
    WavReader reader( "D:/rec5.wav" );
    WavWriter writer( "d:/1.wav", reader.sample_rate(), reader.num_channels() );
    AudioEffect ae;
    ae.Init( reader.sample_rate(), reader.num_channels(), reader.sample_rate(), reader.num_channels() );
    int frames = reader.sample_rate() / 100 * reader.num_channels();
    int16_t* buf = new int16_t[frames];
    for ( ;; )
    {
        if ( frames != reader.ReadSamples( frames, buf ))
             break;
        ae.ProcessCaptureStream( buf, frames*2 );
        while (ae.GetRecordingData(buf,frames*2,false))
        {
            writer.WriteSamples( buf, frames );
        }
    }
    delete[] buf;
}
#include "codec/aac/libAACenc/include/aacenc_lib.h"
void test_aac()
{
    WavReader reader( "E:\\CloudMusic\\Mariage.wav" );
    int channel = reader.num_channels();
    int samplerate = reader.sample_rate();
    /* encoder handle */
    HANDLE_AACENCODER hAacEncoder = NULL;
    AACENC_ERROR ErrorStatus;
    if ( ( ErrorStatus = aacEncOpen( &hAacEncoder, 0, 2 ) ) != AACENC_OK )
    {
        return;
    }
    ErrorStatus = aacEncoder_SetParam( hAacEncoder, AACENC_AOT, AOT_SBR );
    ErrorStatus = aacEncoder_SetParam( hAacEncoder, AACENC_BITRATE, 32000 );
    ErrorStatus = aacEncoder_SetParam( hAacEncoder, AACENC_SAMPLERATE, samplerate );
    ErrorStatus = aacEncoder_SetParam( hAacEncoder, AACENC_CHANNELMODE, MODE_2 );
    ErrorStatus = aacEncEncode( hAacEncoder, NULL, NULL, NULL, NULL );
    AACENC_InfoStruct encInfo;
    ErrorStatus = aacEncInfo( hAacEncoder, &encInfo );
    

    int frames = encInfo.frameLength*channel;
    int16_t* buf = new int16_t[frames];
    char*out_stream = new char[encInfo.maxOutBufBytes];
    
    AACENC_BufDesc encinBuf;
    int in_eisize = 2;
    int in_bufsize = frames*sizeof(int16_t);
    int in_buf_id = IN_AUDIO_DATA;



    encinBuf.bufElSizes = &in_eisize;
    encinBuf.bufferIdentifiers = &in_buf_id;
    encinBuf.bufs = (void**)&buf;
    encinBuf.bufSizes = &in_bufsize;
    encinBuf.numBufs = 1;
    
    AACENC_BufDesc encoutBuf;
    int out_eisize = 2;
    int out_bufsize = encInfo.maxOutBufBytes;
    int out_buf_id = OUT_BITSTREAM_DATA;
    encoutBuf.bufElSizes = &out_eisize;
    encoutBuf.bufferIdentifiers = &out_buf_id;
    encoutBuf.bufSizes = &out_bufsize;
    encoutBuf.numBufs = 1;
    encoutBuf.bufs = (void **)&out_stream;

    AACENC_InArgs in_args;
    AACENC_OutArgs out_args;
    in_args.numAncBytes = 0;
    FILE*outfile = fopen( "D:/Mariage.aac", "wb" );
    for ( ;; )
    {
        auto len = reader.ReadSamples( frames, buf );
        if ( 0 == len )
        {
            encinBuf.bufs = nullptr;
        }
        in_args.numInSamples = (len) * sizeof( int16_t ) / channel;
        auto ret = aacEncEncode( hAacEncoder, &encinBuf, &encoutBuf, &in_args, &out_args );
        if (ret != AACENC_OK)
        {
            break;
        }
        if ( out_args.numOutBytes > 0 )
        {
            static int i = 0;
            printf("[%d]enc out bytes=%d\n",++i,out_args.numOutBytes);
           fwrite( out_stream, 1, out_args.numOutBytes, outfile);
        }
        if (len == 0)
        {
            break;
        }
    }
    delete[] buf;
    delete[] out_stream;
    aacEncClose( &hAacEncoder );
    fclose( outfile );
    outfile = nullptr;
    
}

int main( int argc, char** argv )
{
   // test_windows_core_audio();
   // test_conv();
   // test_hrtf(45,0,"C:/Users/zhangnaigan/Desktop/3D_test_Audio/es01.wav","D:/pro-48000-1.wav");
   // test_real_time_3d();
   // test_mit_hrtf_get();
    //test_circular_buffer();
    //test_play_mp3();
   // test_vcl( argc, argv );
   // test_audio_processing();

    test_aac();
    system( "pause" );
    return 0;

}