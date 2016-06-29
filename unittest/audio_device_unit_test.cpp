#include <iostream>
#include <list>
#include <mutex>
#include <algorithm>
#include "../device/include/audio_device.h"
#include "../effect/3d/include/mit_hrtf_lib.h"
#include "../effect/3d/include/fft.h"

#include <assert.h>

#pragma comment(lib,"../build/winx/Debug/audio_device.lib")
#pragma comment(lib,"../build/winx/Debug/audio_effect.lib")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "msdmo")
#pragma comment(lib, "dmoguids")
#pragma comment(lib, "wmcodecdspuuid")
using namespace std;
typedef lock_guard<mutex> lockguard;

// 卷积公式
//y(n)=sum(x(k)h(n-k)), 其中 n-k>0,0<n<N,0<k<M
// 参见《信号与系统》第50页
void convolution( int16_t *x, int16_t* h, int16_t *y, size_t xn, size_t hn, size_t yn )
{
    memset( y, 0, yn*sizeof( int16_t ) );
    for ( size_t n = 0; n < yn; n++ )
        for ( size_t j = 0; j < xn; j++ )
            if ( n -j >= 0 && n - j < hn )
                y[n] += x[j] * h[n - j];
}
void convolution( const float*input, complex*irc, float *output, int nFFT, int nFil, int nSig )
{
    complex*inc, *outc;
    inc = new complex[nFFT];
    for ( int i = 0; i < nFFT; i++ )
    {
        if ( i < nSig )
        {
            inc[i] = complex( (double)input[i] );
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
        outc[i] = complex( (double)( inc[i].re() * irc[i].re() -
                           inc[i].im() * irc[i].im() ),
                           (double)( inc[i].re() * irc[i].im() +
                           inc[i].im() * irc[i].re() ));
    }

    CFFT::Inverse( outc, nFFT, true );

    for ( int i = 0; i < nFFT; i++ )
    {
        output[i] = (float)( outc[i].re() );
    }
}

class AudioSampleBuffer
{
public:
    AudioSampleBuffer(int samplerate, int channel) :m_samplerate(samplerate), m_channel(channel) {}
    ~AudioSampleBuffer() {}
    const float* getReadPointer( int index ) { return nullptr; }
    float* getWritePointer( int index ) { return nullptr; }
    int getNumSamples()
    {
        return 0;
    }
private:
    int m_samplerate;
    int m_channel;
};


class CAudioBufferProc : public  AudioBufferProc
{
    bool m_processhrtf;
    int16_t* m_pLeft = 0;
    int16_t* m_pRight = 0;
    int m_nTaps = 0;
    int16_t*m_prevLeftData;
    int16_t*m_prevRightData;
    int16_t*m_LeftData;
    int16_t*m_RightData;
    int16_t* m_pInputData;
public:
    CAudioBufferProc(bool processhrtf) :m_processhrtf(processhrtf)
    {
        if (m_processhrtf)
        {
            int nAzimuth = 45;
            int nElevation = 0;
            m_nTaps = mit_hrtf_availability( nAzimuth, nElevation, 48000, 0 );
            if ( m_nTaps )
            {
                m_pLeft = (short int*)malloc( m_nTaps * sizeof( short ) );
                m_pRight = (short int*)malloc( m_nTaps * sizeof( short ) );
                m_nTaps = mit_hrtf_get( &nAzimuth, &nElevation, 48000, 1, m_pLeft, m_pRight );
                m_prevLeftData = new int16_t[480];
                m_prevRightData = new int16_t[480];
                m_pInputData = new int16_t[480];
                m_LeftData = new int16_t[960];
                m_RightData = new int16_t[960];
                memset( m_prevLeftData, 0, 960 );
                memset( m_prevRightData, 0, 960 );
                memset( m_LeftData, 0, 960*2 );
                memset( m_RightData, 0, 960*2 );
            }
        }

    }
    ~CAudioBufferProc()
    {
        free( m_pLeft );
        free( m_pRight );
        delete[] m_prevLeftData;
        delete[] m_prevRightData;
        delete[] m_pInputData;
        delete[] m_LeftData;
        delete[] m_RightData;
    }

    virtual void RecordingDataIsAvailable( const void*data, size_t samples )
    {
        int16_t* pData = ( int16_t* )new char[samples];
        if (m_processhrtf)
        {
            int16_t*p = (int16_t*)data;
            for ( size_t i = 0; i < samples / 2;i++ )
            {
                m_pInputData[i] = p[i*2]; //left channel
            }
            convolution( m_pInputData, m_pLeft, m_LeftData, 480, m_nTaps, 960 );
            convolution( m_pInputData, m_pRight, m_RightData, 480, m_nTaps, 960 );

            for ( int i = 0; i < 480; i++ )
            {
                pData[i * 2] = (m_LeftData[i] + m_prevLeftData[i])/2;
                pData[i * 2 + 1] = (m_RightData[i] + m_prevRightData[i])/2;
                m_prevLeftData[i] = m_LeftData[i + 480];
                m_prevRightData[i] = m_RightData[i + 480];
            }
        }
        else
        {
            memcpy( pData, data, samples );
        }

        lockguard lg( m_lock );
        m_list.push_back((char*)pData);
    }

    virtual size_t NeedMorePlayoutData( void* data, size_t samples )
    {
        lockguard lg( m_lock );
        if (m_list.size() < 50)
        {
            memset( data, 0, samples );

        }
        else
        {
            char* p = m_list.front();
            memcpy( data, p, samples );
            m_list.pop_front();
            delete[] p;
        }
        return samples;
    }
    virtual void ErrorOccurred(AudioError aeCode) {}

    void processBlock( AudioSampleBuffer & buffer )
    {

    }
private:
    list<char*> m_list;
    mutex   m_lock;
};
void test_conv()
{
    int16_t x[3] = { 1, 2, 3 };
    int16_t h[5] = { 1, 3, 5, 7, 9 };
    int16_t y[10] = { 0 };
    convolution( x, h, y, 3, 5, 10 );
    int16_t res[10] = {1,5,14,26,38,39,27,0,0,0};
    assert( !memcmp( y, res, sizeof( int16_t ) * 10 ) );
    for ( int i = 0; i < 10; i++ )
    {
        std::cout << y[i] << "\t";
    }
    std::cout << endl;
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

void test_hrtf()
{
    int nAzimuth = 90;
    int nElevation = 0;
    int nTaps = 0;
    short int* pLeft = 0;
    short int* pRight = 0;

    nTaps = mit_hrtf_availability( nAzimuth, nElevation, 44100, 0 );
    if ( nTaps )
    {
        pLeft = (short int*)malloc( nTaps * sizeof( short ) );
        pRight = (short int*)malloc( nTaps * sizeof( short ) );

        nTaps = mit_hrtf_get( &nAzimuth, &nElevation, 44100, 0, pLeft, pRight );
    }


}


int main( int argc, char** argv )
{
    test_windows_core_audio();
    system( "pause" );
    return 0;

}