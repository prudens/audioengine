//typedef std::complex<float> complex;
// 卷积公式
//y(n)=sum(x(k)h(n-k)), 其中 n-k>0,0<n<N,0<k<M
// 参见《信号与系统》第50页
#include "header.h"
#include "io/include/audioencoder.h"
#include "io/include/audiodecoder.h"

class CAudioBufferProc : public  AudioBufferProc
{
    bool m_processhrtf;
    list<char*> m_list;
    mutex   m_lock;
    Mixer3D m_Mixer3D;
    AudioEffect* pEffect;
public:
    CAudioBufferProc( bool processhrtf, 
                      uint32_t rec_sample_rate,
                      uint16_t rec_channel,
                      uint32_t ply_sample_rate,
                      uint16_t ply_channel )
        :m_processhrtf( processhrtf ), m_Mixer3D( 48000 )
    {
        int nAzimuth = 90;
        int nElevation = 0;
        m_Mixer3D.updateAngles( nAzimuth, nElevation );
        pEffect = new AudioEffect;
        pEffect->Init( rec_sample_rate, rec_channel, ply_sample_rate, ply_channel );
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

void test_windows_core_audio()
{
    AudioDevice* pWinDevice = AudioDevice::Create();

    pWinDevice->Initialize();
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();
    uint32_t rec_sample_rate,ply_sample_rate;
    uint16_t rec_channel,ply_channel;
    pWinDevice->GetRecordingFormat(rec_sample_rate,rec_channel);
    pWinDevice->GetPlayoutFormat( ply_sample_rate, ply_channel );
    CAudioBufferProc cb( false,rec_sample_rate,rec_channel,ply_sample_rate,ply_channel );
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

void test_real_time_3d()
{
    int nAzimuth = 0; int nElevation = 0;
    AudioDevice* pWinDevice = AudioDevice::Create();
    pWinDevice->Initialize();
    pWinDevice->SetRecordingFormat( 48000, 2 );
    pWinDevice->SetPlayoutFormat( 48000, 2 );
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();
    uint32_t rec_sample_rate, ply_sample_rate;
    uint16_t rec_channel, ply_channel;
    pWinDevice->GetRecordingFormat( rec_sample_rate, rec_channel );

    pWinDevice->GetPlayoutFormat( ply_sample_rate, ply_channel );
    CAudioBufferProc cb( true, rec_sample_rate, rec_channel, ply_sample_rate, ply_channel );
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



class OpusBufferProc : public AudioBufferProc
{
public:
    OpusBufferProc()
    {
        encoder_ = AudioEncoder::Create( AFT_OPUS,48000,2,24000 );
        decoder_ = AudioDecoder::Create( AFT_OPUS );
    }
    virtual void RecordingDataIsAvailable( const void*data, size_t size_in_byte )
    {
        std::pair<char*, int> frame;
        frame.first = new char[512];
        frame.second = 512;
        if ( encoder_->Encode( (int16_t*)data, size_in_byte / 2, frame.first, frame.second ) )
        {
            std::lock_guard<std::mutex> lg(lock_);
            buf_list_.push_back( frame );
            printf("encode len = %d\n",frame.second);
        }
        else
        {
            printf("编码失败");
            delete frame.first;
        }

    }
    virtual size_t NeedMorePlayoutData( void*data, size_t size_in_byte )
    {
        std::lock_guard<std::mutex> lg( lock_ );
        if (buf_list_.size() > 50)
        {
            auto frame = buf_list_.front();
            int size;
            if ( !decoder_->Decode( frame.first, frame.second, (int16_t*)data, size ) )
            {
                printf( "解码失败" );
            }
            buf_list_.pop_front();
            delete frame.first;
            size_in_byte = size*2;
        }

        return size_in_byte;
    }
private:
    AudioEncoder* encoder_;
    AudioDecoder* decoder_;
    std::mutex lock_;
    std::list < std::pair<char*, int> > buf_list_;
};
void test_opus_codec()
{
    AudioDevice* pWinDevice = AudioDevice::Create();

    pWinDevice->Initialize();
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();

    OpusBufferProc cb ;
    pWinDevice->SetAudioBufferCallback( &cb );
    pWinDevice->StartPlayout();
    pWinDevice->StartRecording();

    system( "pause" );

    pWinDevice->StopRecording();
    pWinDevice->StopPlayout();
    pWinDevice->Terminate();
    pWinDevice->Release();
}

void test_audio_device()
{
    test_windows_core_audio();
    //test_opus_codec();
}