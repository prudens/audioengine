#include <iostream>
#include <list>
#include <mutex>
#include <algorithm>
#include <cassert>
#include <complex>

#include "base/fft.h"
#include "base/audio_util.h"
#include "device/include/audio_device.h"
#include "effect/3d/include/mit_hrtf_lib.h"
#include "io/wav_file.h"
#include "base/circular_buffer.hpp"

#pragma comment(lib,"../build/winx/Debug/audio_device.lib")
#pragma comment(lib,"../build/winx/Debug/audio_effect.lib")
#pragma comment(lib,"../build/winx/Debug/audio_io.lib")
#pragma comment(lib,"../build/winx/Debug/audio_base.lib")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "msdmo")
#pragma comment(lib, "dmoguids")
#pragma comment(lib, "wmcodecdspuuid")
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
#include <complex>
class AudioSampleBuffer
{
public:
    AudioSampleBuffer(short*pData,int nSamples)
    {
        NumSamples = nSamples;
        m_pData[0] = new float[nSamples];
        m_pData[1] = new float[nSamples];
        if (pData)
        {
            for ( int i = 0; i < nSamples; i++ )
            {
                m_pData[0][i] = S16ToFloat(pData[i * 2]);
                m_pData[1][i] = S16ToFloat(pData[i * 2 + 1]);
            }
        }
        else
        {
            memset( m_pData[0], 0, nSamples*sizeof( float ) );
            memset( m_pData[1], 0, nSamples*sizeof( float ) );
        }
    }
    ~AudioSampleBuffer() {
        delete[] m_pData[0];
        delete[] m_pData[1];

    }
    const float* getReadPointer( int index )
    {
        return m_pData[index];
    }
    float* getWritePointer( int index ) 
    {
        return m_pData[index];
    }

    int getNumSamples()
    {
        return NumSamples;
    }
private:
    float* m_pData[2];
    int NumSamples;
};


class CAudioBufferProc : public  AudioBufferProc
{
    AudioSampleBuffer proBuf;
    bool m_processhrtf;
    CircularAudioBuffer m_audio_buffer;
    CircularAudioBuffer m_audio_buffer_out;
    static const int nFFT = 1024;
    complex m_fltl[nFFT];
    complex m_fltr[nFFT];
    list<char*> m_list;
    mutex   m_lock;

    int nFil;
    int nSig;
    WavWriter writer_pro;
    WavWriter writer_src;
public:
    ~CAudioBufferProc()
    {
    }
    CAudioBufferProc(bool processhrtf) :m_processhrtf(processhrtf), proBuf(nullptr,nFFT/2)
        ,m_audio_buffer(nFFT*2)
        , m_audio_buffer_out(nFFT*2)
        ,writer_pro(("D:/pro-48000-2.wav"),48000,2)
        , writer_src("D:/src-48000-2.wav",48000,2)
    {
         int nAzimuth = 90;
         int nElevation = 0;
        if (m_processhrtf)
        {
            float pLeft[nFFT] = { 0 };
            float pRight[nFFT] = { 0 };
            int nFil = mit_hrtf_get( &nAzimuth, &nElevation, 48000, 0, pLeft, pRight );
            if ( nFil == 0 ) throw std::invalid_argument( "nAzimuth and nElevation is invalid!" );
            for ( int i = 0; i < nFFT; i++ )
            {
                if (i < nFil)
                {
                    m_fltl[i] = complex( pLeft[i] );
                    m_fltr[i] = complex( pRight[i] );
                }
                else
                {
                    m_fltl[i] = complex( 0 );
                    m_fltr[i] = complex( 0 );
                }
            }
            CFFT::Forward( m_fltl, nFFT );
            CFFT::Forward( m_fltr, nFFT );
        }
    }

    virtual void RecordingDataIsAvailable( const void*data, size_t samples )
    {
        lockguard lg( m_lock );
        int16_t* pData = ( int16_t* )new int16_t[nFFT];
        if (m_processhrtf)
        {
            m_audio_buffer.write( (int16_t*)data, samples/2);
            if ( m_audio_buffer.readSizeRemaining() < nFFT )
            {
                return;
            }
            int16_t buf[nFFT];
            size_t readlen = m_audio_buffer.read( (int16_t*)buf, nFFT );
            assert( readlen == nFFT );
            AudioSampleBuffer buffer( buf, readlen/2 );
            processBlock( buffer );
            const float*pLeft = buffer.getReadPointer( 0 );
            const float*pRight = buffer.getReadPointer( 1 );
            
            for ( size_t i = 0; i < readlen / 2; i++ )
            {
                pData[i * 2] = FloatToS16( pLeft[i] );
                pData[i * 2 + 1] = FloatToS16( pRight[i] );
            }
        }
        else
        {
            memcpy( pData, data, samples );
        }

        m_list.push_back((char*)pData);
    }

    virtual size_t NeedMorePlayoutData( void* data, size_t samples )
    {
        lockguard lg( m_lock );

        for ( ;; )
        {
            if ( samples / 2 < m_audio_buffer_out.readSizeRemaining() )
            {
                assert(samples/2 == m_audio_buffer_out.read( (int16_t*)data, samples / 2 ));
                return samples;
            }
            if ( m_list.size() < 50 )
            {
                memset( data, 0, samples );
                return samples;
            }
            char* p = m_list.front();
            m_list.pop_front();
            m_audio_buffer_out.write( (int16_t*)p, nFFT );
            delete[] p;
        }
        return samples;
    }

    virtual void ErrorOccurred(AudioError aeCode) {}

    void processBlock( AudioSampleBuffer & buffer )
    {
        const float*inL = buffer.getReadPointer( 0 );
        const float*inR = buffer.getReadPointer( 1 );
        float *outL = buffer.getWritePointer( 0 );
        float *outR = buffer.getWritePointer( 1 );

        int bufSize = buffer.getNumSamples();

        const float *proRdL = proBuf.getReadPointer( 0 );
        const float * proRdR = proBuf.getReadPointer( 1 );
        float *proWtL = proBuf.getWritePointer( 0 );
        float *proWtR = proBuf.getWritePointer( 1 );

        nSig = bufSize;
        float *outLp = new float[nFFT];
        float *outRp = new float[nFFT];
        convolution( inL, m_fltl, outLp, nFFT, nFil, nSig );
        convolution( inR, m_fltr, outRp, nFFT, nFil, nSig );
        for ( int i = 0; i < bufSize; i++ )
        {
            outL[i] = ( outLp[i] + proRdL[i] ) / 2;
            outR[i] = ( outRp[i] + proRdR[i] ) / 2;
            proWtL[i] = outLp[i + bufSize];
            proWtR[i] = outRp[i + bufSize];
        }
        delete[] outLp;
        delete[] outRp;
    }
private:

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
    complex data[] = { 1, 2, 3,4 };
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
    pWinDevice->SetRecordingFormat( 48000, 2 );
    pWinDevice->SetPlayoutFormat( 48000, 2 );
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
    AudioDevice* pWinDevice = AudioDevice::Create();
    pWinDevice->Initialize();
    pWinDevice->SetRecordingFormat( 48000, 2 );
    pWinDevice->SetPlayoutFormat( 48000, 2 );
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();

    CAudioBufferProc cb( true );
    pWinDevice->SetAudioBufferCallback( &cb );
    pWinDevice->StartPlayout();
    pWinDevice->StartRecording();

    system( "pause" );

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
//         S16ToFloat( irL_, nFil, flt );
//         S16ToFloat( irR_, nFil, frt );
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


int main( int argc, char** argv )
{
   // test_windows_core_audio();
   // test_conv();
   // test_hrtf(45,0,"C:/Users/zhangnaigan/Desktop/3D_test_Audio/es01.wav","D:/pro-48000-1.wav");
    test_real_time_3d();
 //   test_mit_hrtf_get();
    test_circular_buffer();
    system( "pause" );
    return 0;

}