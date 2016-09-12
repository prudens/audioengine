//typedef std::complex<float> complex;
// 卷积公式
//y(n)=sum(x(k)h(n-k)), 其中 n-k>0,0<n<N,0<k<M
// 参见《信号与系统》第50页
#include "header.h"

template <class T>
void conv( T *x, T* h, T *y, size_t xn, size_t hn, size_t yn )
{
    memset( y, 0, sizeof( T ) * yn );
    for ( size_t n = 0; n < yn; n++ )
        for ( size_t j = 0; j < xn; j++ )
            if ( n - j >= 0 && n - j < hn )
                y[n] += x[j] * h[n - j];
}

#define TEST_CONV
void convolution( const float*input, Complex*irc, float *output, int nFFT, int nFil, int nSig )
{
#ifdef TEST_CONV
    for ( int i = 0; i < nFFT; i++ )
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
    CAudioBufferProc( bool processhrtf ) :m_processhrtf( processhrtf ), m_Mixer3D( 48000 )
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

        if ( !pEffect->HasVoice() )
        {
            //printf( "cur frame is silent\n" );
        }
        if ( m_processhrtf )
        {
            m_Mixer3D.AddProcessData( (int16_t*)data, samples / 2 );
            for ( ;; )
            {
                char* pData = new char[samples];
                size_t readsample = m_Mixer3D.GetProcessData( (int16_t*)pData, samples / 2 );
                if ( readsample == 0 )
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
            if ( !pEffect->HasVoice() )
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
        if ( m_list.size() < 50 )
        {
            return samples;
        }

        char* p = m_list.front();
        m_list.pop_front();
        memcpy( data, p, samples );
        delete p;
        return samples;
    }

    virtual void ErrorOccurred( AudioError aeCode ) {}
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
    int16_t res[10] = { 1, 5, 14, 26, 38, 39, 27, 0, 0, 0 };
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
        assert( data[i].real() == i + 1 );
    }

    //test convolution

}

void test_windows_core_audio()
{
    AudioDevice* pWinDevice = AudioDevice::Create();

    pWinDevice->Initialize();
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();

    CAudioBufferProc cb( false );
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
    float pLeft[nFFT] = { 0 };
    float pRight[nFFT] = { 0 };
    int nFil = mit_hrtf_get( &nAzimuth, &nElevation, reader.sample_rate(), 0, pLeft, pRight );
    if ( nFil == 0 ) return;
    Complex flt[nFFT] = { 0 };
    for ( int i = 0; i < nFil; i++ )
    {
        flt[i] = Complex( pLeft[i] );

    }
    CFFT::Forward( flt, nFFT );

    WavWriter writer( outputfile, reader.sample_rate(), 1 );

    int16_t pSrc[nFFT * 2];
    reader.ReadSamples( nFFT*reader.num_channels(), pSrc );
    reader.ReadSamples( nFFT*reader.num_channels(), pSrc );
    reader.ReadSamples( nFFT*reader.num_channels(), pSrc );
    int16_t pMono[nFFT];
    DownmixInterleavedToMono( pSrc, nFFT, reader.num_channels(), pMono );
    writer.WriteSamples( pMono, nFFT );
    float pData[nFFT];
    S16ToFloat( pMono, nFFT, pData );
    float output[nFFT];
    convolution( pData, flt, output, nFFT, nFil, nFFT );
    FloatToS16( output, nFFT, pSrc );

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

    CAudioBufferProc cb( true );
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
                printf( "invalid argment:%c", ch );
            }

            nAzimuth = (std::min )( (std::max) ( nAzimuth, -180 ), 180 );
            nElevation = (std::min) ( (std::max) ( nElevation, -40 ), 90 );
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
    CircularAudioBuffer buffer( capacity );
    assert( 0 == buffer.readSizeRemaining() );
    assert( 10 == buffer.writeSizeRemaining() );
    int16_t data[4] = { 1, 2, 3, 4 };
    assert( 4 == buffer.write( data, 4 ) );
    assert( 6 == buffer.writeSizeRemaining() );
    assert( 4 == buffer.readSizeRemaining() );
    int16_t ReadData[4] = { 0 };
    assert( 4 == buffer.read( ReadData, 4 ) );
    assert( !memcmp( ReadData, data, 4 * sizeof( int16_t ) ) );
    assert( 0 == buffer.read( ReadData, 3 ) );
    int16_t data10[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    assert( 10 == buffer.write( data10, 10 ) );
    assert( 0 == buffer.write( data, 4 ) );
}