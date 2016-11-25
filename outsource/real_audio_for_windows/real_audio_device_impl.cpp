#include "real_audio_device_interface.h"
#include <atomic>
#include <string>
#include <list>
#include <mutex>
#include <memory>
#include <vector>
#include "device/include/audio_device.h"
#include "audio_effect.h"
#include "base/time_cvt.hpp"

class CAudioBufferProc : public  AudioBufferProc
{
public:
    typedef std::lock_guard<std::mutex> lockguard;
    std::mutex   m_lock;
    AudioEffect* pEffect;
    LPRECORDINGDATACALLBACK RecordingData=NULL;
    size_t frame_size_;
    std::vector<char> rec_cache_;
    std::vector<char> ply_cache_;
    FILE* file1;
    FILE* file2;
    std::atomic<int> count_;
public:
    CAudioBufferProc( uint32_t rec_sample_rate,
                      uint16_t rec_channel,
                      uint32_t ply_sample_rate,
                      uint16_t ply_channel )
    {
        pEffect = new AudioEffect;
        pEffect->RecordingReset(rec_sample_rate, rec_channel, AudioEffect::kTargetRecSampleRate, rec_channel );
        pEffect->PlayoutReset( AudioEffect::kTargetPlySampleRate, ply_channel, ply_sample_rate, ply_channel );
        frame_size_ = 320 * rec_channel;// д╛хо10ms
        file1 = fopen( "d:/dmo.pcm","wb+" );
        file2 = fopen( "d:/dmo2.pcm", "wb+" );
        count_ = 1;

    }
    ~CAudioBufferProc()
    {
        delete pEffect;
        fclose( file1 );
        fclose( file2 );

    }

    virtual void RecordingDataIsAvailable( const void*data, size_t samples )
    {
        count_++;
//        static int data_len = 0;
//         data_len += samples;
//         static uint64_t ts = timestamp();
//         if (count_  == 500)
//         {
//             auto t = timestamp() - ts;
//             printf( "[%I64u] RecordingDataIsAvailable %d, data_len = %d \n", timestamp(), data_len / (int)t, data_len );
//             count_ = 0;
//             data_len = 0;
//             ts = timestamp();
//         }
        printf( "[%I64u] RecordingDataIsAvailable %d  \n", timestamp(), count_ );
       // printf( "samples = %d\n",samples );
        fwrite( data,1, samples, file2 );
       // return;
        if ( !RecordingData )
        {
            return;
        }
        {
            size_t outSize = samples;
            pEffect->ProcessCaptureStream( (int16_t*)data, samples, (int16_t*)data, outSize );
            rec_cache_.insert( rec_cache_.end(), (char*)data, (char*)data + outSize );  
            
            if ( rec_cache_.size() >= frame_size_ )
            {

                RecordingData( rec_cache_.data(), frame_size_ );
                fwrite( rec_cache_.data(), 1, frame_size_, file1 );
                if ( rec_cache_.size() == frame_size_ )
                {
                    rec_cache_.clear();
                }
                else
                {
                    rec_cache_.erase( rec_cache_.begin(), rec_cache_.begin() + frame_size_ );
                }
            }
            else
            {
                return;
            }
        }
    }

    virtual size_t NeedMorePlayoutData( void* data, size_t len_of_byte )
    {
        count_--;
        printf( "[%I64u] NeedMorePlayoutData %d  \n", timestamp(), count_ );
        lockguard lg( m_lock );
        size_t frame_size = pEffect->ply_resample.frame_size;
        if ( ply_cache_.size() >= frame_size )
        {
            
            pEffect->ProcessRenderStream( (int16_t*)ply_cache_.data(), frame_size, (int16_t*)data, len_of_byte );

            ply_cache_.erase( ply_cache_.begin(), ply_cache_.begin() + frame_size );
        }
        return len_of_byte;
    }

    virtual void ErrorOccurred( AudioError aeCode ) {}

    void FillPlayoutData(const void* pcm16_data, int len_of_byte)
    {
        if (len_of_byte == 0 || pcm16_data == nullptr)
        {
            return;
        }
        fwrite( pcm16_data, 1, len_of_byte, file2 );
        lockguard lg( m_lock );
        ply_cache_.insert(ply_cache_.end(), (char*)pcm16_data, (char*)pcm16_data + len_of_byte );
    }

    void SetFrameSize(size_t len_of_byte)
    {
        frame_size_ = len_of_byte;
        rec_cache_.reserve( frame_size_ );
        ply_cache_.reserve( frame_size_ );
    }

    size_t GetFrameSize()
    {
        return frame_size_;
    }
};

struct RealAudioDevice
{
    AudioDevice* pWinDevice;
    CAudioBufferProc* pAudioBufferProc;
    int32_t rec_sample_rate;
    int16_t rec_channel;
    int32_t ply_sample_rate; 
    int16_t ply_channel;
};

DID REAL_AUDIO_CALL CreateDevice()
{
    RealAudioDevice* pInstance = new RealAudioDevice;
    AudioDevice* pWinDevice = AudioDevice::Create();
    pWinDevice->Initialize();
    int32_t v = 1;
    pWinDevice->SetPropertie( ID_ENABLE_AEC, &v );
    pWinDevice->SetPropertie( ID_ENBALE_NS, &v );
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();
    uint32_t rec_sample_rate, ply_sample_rate;
    uint16_t rec_channel, ply_channel;
    pWinDevice->GetRecordingFormat( rec_sample_rate, rec_channel );
    pWinDevice->GetPlayoutFormat( ply_sample_rate, ply_channel );

    CAudioBufferProc* proc = new CAudioBufferProc( rec_sample_rate, rec_channel,
                                                   ply_sample_rate, ply_channel );
    pWinDevice->SetAudioBufferCallback( proc );
    pInstance->pWinDevice = pWinDevice;
    pInstance->pAudioBufferProc = proc;
    return (DID)pInstance;
}

void REAL_AUDIO_CALL DestroyDevice( DID device_id )
{
    if ( device_id == 0 ) return;
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;
    if (!pInstance->pWinDevice)
    {
        return;
    }
    pInstance->pWinDevice->Terminate();
    pInstance->pWinDevice = nullptr;
    delete pInstance->pAudioBufferProc;
}

void REAL_AUDIO_CALL SetSampleRate( DID device_id, int32_t rec_sample_rate, int16_t rec_channel, int32_t ply_sample_rate, int16_t ply_channel )
{
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;
    if ( 0 == device_id )
    {
        return;
    }
    auto pEffect = pInstance->pAudioBufferProc->pEffect;
    uint32_t sample_rate;
    uint16_t channel;
    pInstance->pWinDevice->GetRecordingFormat( sample_rate ,channel);
    pEffect->RecordingReset( sample_rate, channel, rec_sample_rate, rec_channel );
    pInstance->pWinDevice->GetPlayoutFormat( sample_rate, channel );
    pEffect->PlayoutReset( ply_sample_rate, ply_channel, sample_rate, channel );
    pInstance->rec_sample_rate = rec_sample_rate;
    pInstance->rec_channel = rec_channel;
    pInstance->ply_sample_rate = ply_sample_rate;
    pInstance->ply_channel = ply_channel;
}

void REAL_AUDIO_CALL GetSampleRate( DID device_id,
                                    int32_t* rec_sample_rate, 
                                    int16_t* rec_channel,
                                    int32_t* ply_sample_rate,
                                    int16_t* ply_channel )
{
    if ( 0 == device_id )
    {
        return;
    }
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;

    *rec_sample_rate = pInstance->rec_sample_rate;
    *rec_channel = pInstance->rec_channel;
    *ply_sample_rate = pInstance->ply_sample_rate;
    *ply_channel = pInstance->ply_channel;
}


void REAL_AUDIO_CALL SetRecordingDataCallback( DID device_id, LPRECORDINGDATACALLBACK  cb )
{
    if ( 0 == device_id )
    {
        return;
    }
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;
    pInstance->pAudioBufferProc->RecordingData = cb;
}

void REAL_AUDIO_CALL FillPlayoutData( DID device_id, const void*pcm16_data, int len_of_byte )
{
    if ( 0 == device_id )
    {
        return;
    }
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;
    pInstance->pAudioBufferProc->FillPlayoutData( pcm16_data, len_of_byte );
}

bool REAL_AUDIO_CALL StartRecording( DID device_id )
{
    if ( 0 == device_id )
    {
        return false;
    }
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;
    return pInstance->pWinDevice->StartRecording();
}

void REAL_AUDIO_CALL StopRecording( DID device_id )
{
    if ( 0 == device_id )
    {
        return;
    }
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;
    pInstance->pWinDevice->StopRecording();
}

bool REAL_AUDIO_CALL StartPlayout( DID device_id )
{
    if ( 0 == device_id )
    {
        return false;
    }
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;

    return pInstance->pWinDevice->StartPlayout();
}

void REAL_AUDIO_CALL StopPlayout( DID device_id )
{
    if ( 0 == device_id )
    {
        return;
    }
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;

    pInstance->pWinDevice->StopPlayout();
}

void REAL_AUDIO_CALL SetFrameSize( DID device_id, int32_t len_of_byte )
{
    if ( 0 == device_id )
    {
        return;
    }
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;

    pInstance->pAudioBufferProc->SetFrameSize( len_of_byte );
}

int32_t REAL_AUDIO_CALL GetFrameSize( DID device_id )
{
    if ( 0 == device_id )
    {
        return 0 ;
    }
    RealAudioDevice* pInstance = (RealAudioDevice*)device_id;

    return pInstance->pAudioBufferProc->GetFrameSize();
}