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

//#define TEST_CONV
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
                m_pData[0][i] = pData[i * 2];
                m_pData[1][i] = pData[i * 2 + 1];
            }
        }
        else
        {
            memset( m_pData[0], 0, nSamples*sizeof( float ) );
            memset( m_pData[1], 0, nSamples*sizeof( float ) );
        }
    }
    ~AudioSampleBuffer() {}
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
    bool m_processhrtf;
    short* irL_;
    short* irR_;
    int16_t* pBigData;
    WavWriter* writer_48000_1;;
    WavWriter* writer_48000_2;
public:
    ~CAudioBufferProc()
    {
        delete[] irLc;
        delete[] irRc;
        delete writer;
        delete writer_48000_1;
        delete writer_48000_2;
    }
    CAudioBufferProc(bool processhrtf) :m_processhrtf(processhrtf), proBuf(nullptr,480)
    {
        std::string filename = "D:/audio-48000.wav";
        writer = new WavWriter(filename,48000,2);
        filename = "D:/audio-48000-1.wav";
        writer_48000_1 = new WavWriter( filename, 48000, 1 );
        filename = "D:/audio-48000-2.wav";
        writer_48000_2 = new WavWriter( filename, 48000, 2 );

        if (m_processhrtf)
        {
            nFFT = 480 * 2;
            int nAzimuth = 45;
            int nElevation = 90;
            nFil = mit_hrtf_availability( nAzimuth, nElevation, 48000, 1 );
            if ( nFil )
            {
                irL_ = new short[nFil];
                irR_ = new short[nFil];
                irLc = new complex[nFFT];
                irRc = new complex[nFFT];

                nFil = mit_hrtf_get( &nAzimuth, &nElevation, 48000, 1, irL_, irR_ );

                for ( int i = 0; i < nFFT; i++ )
                {
                    if ( i < nFil )
                    {
                        irLc[i] = complex( (float)irL_[i] );
                        irRc[i] = complex( (float)irR_[i] );
                    }
                    else
                    {
                        irLc[i] = complex( 0 );
                        irRc[i] = complex( 0 );
                    }
                }
                CFFT::Forward( irLc, nFFT );
                CFFT::Forward( irRc, nFFT );
            }
        }

    }

    virtual void RecordingDataIsAvailable( const void*data, size_t samples )
    {
        int16_t* pData = ( int16_t* )new char[samples];
        if (m_processhrtf)
        {
            AudioSampleBuffer buffer( (short*)data, samples/4 );
            processBlock( buffer );
            const float *outL = buffer.getReadPointer( 0 );
            const float *outR = buffer.getReadPointer( 1 );
            for ( size_t i = 0; i < samples / 4; i++ )
            {
                pData[i * 2] = static_cast<int16_t>(outL[i]);
                pData[i * 2 + 1] = static_cast<int16_t>( outR[i] );
                //std::cout << pData[i]<<",";
            }

            int16_t* pTmp = new int16_t[samples/4];
          //  int16_t* outputL = new int16_t[samples/2];
          //  int16_t* outputR = new int16_t[samples / 2];
            for ( size_t i = 0; i < samples/4;i++ )
            {
                pTmp[i] = ((int16_t*)data)[i * 2];
            }
            writer_48000_1->WriteSamples( pTmp, 480 );
        //    conv( pTmp, irL_, outputL, samples/4, nFil, samples / 2 );
       //     conv( pTmp, irR_, outputR, samples/4, nFil, samples / 2 );
//             for ( size_t i = 0; i < samples / 4;i++ )
//             {
//                 pData[i * 2] = outputL[i];
//                 pData[i * 2 + 1] = outputR[i];
//             }

            delete[] pTmp;
          //  delete[] outputL;
          //  delete[] outputR;
        }
        else
        {
            memcpy( pData, data, samples );
        }
        writer_48000_2->WriteSamples( (int16_t*)data, 960 );
        writer->WriteSamples( pData, 480*2 );
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
        const float*inL = buffer.getReadPointer( 0 );
        const float*inR = buffer.getReadPointer( 1 );
        float *outL = buffer.getWritePointer( 0 );
        float *outR = buffer.getWritePointer( 1 );

        int bufSize = buffer.getNumSamples();

        const float *proRdL = proBuf.getReadPointer( 0 );
        const float * proRdR = proBuf.getReadPointer( 1 );
        float *proWtL = proBuf.getWritePointer( 0 );
        float *proWtR = proBuf.getWritePointer( 1 );

        nFFT = 2 * bufSize;
        nSig = bufSize;
        float *outLp = new float[nFFT];
        float *outRp = new float[nFFT];
        convolution( inL, irLc, outLp, nFFT, nFil, nSig );
        convolution( inR, irRc, outRp, nFFT, nFil, nSig );
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
    list<char*> m_list;
    mutex   m_lock;
    AudioSampleBuffer proBuf;
    int nFFT;
    int nFil;
    int nSig;
    complex* irLc;
    complex* irRc;
    WavWriter *writer;
};


void test_conv()
{
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
    complex data[] = { 1, 2, 3 };
    CFFT::Forward( data, 3 );
    CFFT::Inverse( data, 3 );

    for ( int i = 0; i < 3; i++ )
    {
       // cout << data[i].re() << ",";
        assert( data[i].real() == i+1 );
    }



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
    int nFil = mit_hrtf_availability( nAzimuth, nElevation, reader.sample_rate(), 0 );
    int16_t *irL_ = nullptr; 
    int16_t *irR_ = nullptr;
    if ( nFil )
    {
        irL_ = new int16_t[nFil];
        irR_ = new int16_t[nFil];
        nFil = mit_hrtf_get( &nAzimuth, &nElevation, reader.sample_rate(), 0, irL_, irR_ );
    }
    else
    {
        return;
    }
    int len = reader.num_samples();
    len = 100;
    int16_t *pSrc = new int16_t[len+nFil];
    len = reader.ReadSamples( len, pSrc );
    float* fData = new float[len];
    S16ToFloat(pSrc,len,fData);
    float* foutputR = new float[nFil +len];
    float* ffilterR = new float[nFil];
    S16ToFloat( irR_, nFil, ffilterR );
    conv( fData, ffilterR, foutputR, len, nFil, nFil + len );
    FloatToS16( foutputR, nFil + len, pSrc );
    WavWriter writer( outputfile, reader.sample_rate(), 1 );
    writer.WriteSamples( pSrc, nFil + len + 1 );
    for ( int i = 0; i < len; i++ )
    {
        cout << pSrc[i] << '\n';
    }
    delete[] irL_;
    delete[] irR_;
    delete[] pSrc;
    delete[] fData;
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
    system( "pause" );
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();
    pWinDevice->StartPlayout();
    pWinDevice->StartRecording();
    system( "pause" );
    pWinDevice->Terminate();
    pWinDevice->Release();
}

int main( int argc, char** argv )
{
   // test_windows_core_audio();
   // test_conv();
    test_hrtf(45,0,"D:/audio-48000-1.wav","D:/pro-48000-1.wav");

    system( "pause" );
    return 0;

}