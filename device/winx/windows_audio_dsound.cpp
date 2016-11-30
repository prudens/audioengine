#include "windows_audio_dsound.h"
#include <dsound.h>
#include <Strsafe.h>
#include <algorithm>
WindowsAudioDSound::WindowsAudioDSound()
{
    HRESULT hr = S_OK;
    hr = DirectSoundEnumerate( &WindowsAudioDSound::EnumCaptrueCallback, &capture_devices_ );
    hr = DirectSoundCaptureEnumerate( &WindowsAudioDSound::EnumCaptrueCallback, &render_devices_ );
}

WindowsAudioDSound::~WindowsAudioDSound()
{

}

void WindowsAudioDSound::Release()
{
    delete this;
}

bool WindowsAudioDSound::Initialize()
{
    if (render_device_index_ == -1)
    {
        return false;
    }
    GUID guid;
    CLSIDFromString( render_devices_[render_device_index_].szDeviceID, &guid );

    HRESULT hr = S_OK;
    hr = DirectSoundCreate( &guid, &render_direct_sound_, NULL );
    HWND hWnd = GetDesktopWindow();
    hr = render_direct_sound_->SetCooperativeLevel( hWnd, DSSCL_PRIORITY );
    if (FAILED(hr))
    {
        return false;
    }

    return initialize_;
}

void WindowsAudioDSound::Terminate()
{
    if ( !initialize_ )
    {
        return;
    }
    StopRecording();
    StopPlayout();
}

size_t WindowsAudioDSound::GetRecordingDeviceNum() const
{
    return capture_devices_.size();
}

size_t WindowsAudioDSound::GetPlayoutDeviceNum() const
{
    return render_devices_.size();
}

bool WindowsAudioDSound::GetPlayoutDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    std::lock_guard<std::mutex> lg( render_lock_ );

    if ( index >= (int16_t)render_devices_.size() )
    {
        return false;
    }
    if ( index < 0 )
    {
        for ( size_t i = 0; i < render_devices_.size( ); i++)
        {
            if ( render_devices_[i].bDefaultDevice )
            {
                index = i;
                break;
            }
        }
    }

    StringCchCopyW( name, kAdmMaxGuidSize - 1, render_devices_[index].szDeviceName );
    StringCchCopyW( guid, kAdmMaxGuidSize - 1, render_devices_[index].szDeviceID );
    return true;
}

bool WindowsAudioDSound::RecordingDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    std::lock_guard<std::mutex> lg( capture_lock_ );

    if ( index >= (int16_t)capture_devices_.size() )
    {
        return false;
    }
    if ( index < 0 )
    {
        for ( size_t i = 0; i < capture_devices_.size(); i++ )
        {
            if ( capture_devices_[i].bDefaultDevice )
            {
                index = i;
                break;
            }
        }
    }

    StringCchCopyW( name, kAdmMaxGuidSize - 1, capture_devices_[index].szDeviceName );
    StringCchCopyW( guid, kAdmMaxGuidSize - 1, capture_devices_[index].szDeviceID );
    return true;
}

bool WindowsAudioDSound::SetPlayoutDevice( int16_t index )
{
    std::lock_guard<std::mutex>  lg( render_lock_ );
    if ( !initialize_ )
    {
        return false;
    }
    if ( index <= (int16_t)render_devices_.size() )
    {
        return false;
    }

    render_device_index_ = index;
    return true;
}

bool WindowsAudioDSound::SetRecordingDevice( int16_t index )
{
    std::lock_guard<std::mutex>  lg( capture_lock_ );
    if ( !initialize_ )
    {
        return false;
    }
    if ( index <= (int16_t)capture_devices_.size() )
    {
        return false;
    }

    render_device_index_ = index;
    return true;
}

bool WindowsAudioDSound::IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    return true;
}

bool WindowsAudioDSound::IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    return true;
}

bool WindowsAudioDSound::SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )
{

}

bool WindowsAudioDSound::SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    // Get the primary buffer 
    HRESULT hr = S_OK;
    DSBUFFERDESC                 dscbd;
    memset( &dscbd, 0, sizeof( DSBUFFERDESC ) );
    dscbd.dwSize = sizeof( DSBUFFERDESC );
    dscbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dscbd.dwBufferBytes = 0;
    dscbd.lpwfxFormat = NULL;
    LPDIRECTSOUNDBUFFER          pDSBPrimary = NULL;
    hr = render_direct_sound_->CreateSoundBuffer( &dscbd, &pDSBPrimary, NULL );
    if ( FAILED( hr ) )
    {
        return false;
    }
    WAVEFORMATEX wfx;
    wfx.cbSize = 0;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = nChannels;
    wfx.nSamplesPerSec = nSampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.wBitsPerSample * nChannels / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

    int frameSize = wfx.nSamplesPerSec / 100;
    int           framedelay = ( std::max )( 5UL, 1000 * frameSize / wfx.nAvgBytesPerSec );
    unsigned long dwBufferSize = ( std::max )( 2, ( MA_DELAY / framedelay ) ) * frameSize + frameSize;


    hr = pDSBPrimary->SetFormat( &wfx );
    if ( FAILED( hr ) )
    {
        return false;
    }
    if ( pDSBPrimary )
    {
        pDSBPrimary->Release();
        pDSBPrimary = NULL;
    }

    dscbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2
        | DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS;

    dscbd.dwBufferBytes = dwBufferSize;
    dscbd.dwReserved = 0;
    dscbd.lpwfxFormat = &wfx;
    hr = render_direct_sound_->CreateSoundBuffer( &dscbd, &render_direct_sound_buf_, NULL );
    if ( FAILED( hr ) )
    {
        dscbd.dwFlags &= ~DSBCAPS_LOCHARDWARE;
        dscbd.dwFlags |= DSBCAPS_LOCSOFTWARE;
        hr = render_direct_sound_->CreateSoundBuffer( &dscbd, &render_direct_sound_buf_, NULL );

        if ( FAILED( hr ) )
            return false;
    }


    render_channel_ = nChannels;
    render_sample_rate_ = nSampleRate;
    init_playout_ = true;
    return true;
}

bool WindowsAudioDSound::GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    nSampleRate = capture_sample_rate_;
    nChannels = capture_channel_;
    return true;
}

bool WindowsAudioDSound::GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    nSampleRate = render_sample_rate_;
    nChannels = render_channel_;
    return true;
}

bool WindowsAudioDSound::InitPlayout()
{   
    if ( !init_playout_ )
    {
        init_playout_ = SetPlayoutFormat(16000,2);
    }
    return init_playout_;
}

bool WindowsAudioDSound::InitRecording()
{
    return true;
}

bool WindowsAudioDSound::StartPlayout()
{
    if ( !initialize_ )
    {
        return false;
    }

    if ( playout_thread_handle_ != NULL )
    {
        return true;
    }

    if ( playing_ )
    {
        return true;
    }

    {
        std::lock_guard<std::mutex> lg( render_lock_ );
        // Create thread which will drive the rendering.
        playout_thread_handle_ = CreateThread(
            NULL,
            0,
            WSAPIRenderThread,
            this,
            0,
            NULL );
        if ( playout_thread_handle_ == NULL )
        {
            return false;
        }

        // Set thread priority to highest possible.
        ::SetThreadPriority( playout_thread_handle_, THREAD_PRIORITY_TIME_CRITICAL );
    }  // critScoped

    DWORD ret = WaitForSingleObject( wait_recording_thread_start_handle_, 1000 );
    if ( ret != WAIT_OBJECT_0 )
    {
        return false;
    }

    playing_ = true;

    return true;
}

bool WindowsAudioDSound::StopPlayout()
{

}

bool WindowsAudioDSound::Playing() const
{

}

bool WindowsAudioDSound::StartRecording()
{

}

bool WindowsAudioDSound::StopRecording()
{

}

bool WindowsAudioDSound::Recording() const
{

}

void WindowsAudioDSound::SetAudioBufferCallback( AudioBufferProc* pCallback )
{

}

AudioBufferProc* WindowsAudioDSound::GetAudioBufferCallback() const
{

}

bool WindowsAudioDSound::SetPropertie( AudioPropertyID id, void* )
{

}

bool WindowsAudioDSound::GetProperty( AudioPropertyID id, void* )
{

}


BOOL CALLBACK WindowsAudioDSound::EnumCaptrueCallback( GUID* pGUID, LPCSTR szDesc, LPCSTR /*szDrvName*/, void* pContext )
{
    AUDIO_DEVICE_INFO_LIST *devices = (AUDIO_DEVICE_INFO_LIST*)pContext;

    if ( pGUID )
    {
        AUDIO_DEVICE_INFO info;
        StringFromGUID2( *pGUID, info.szDeviceID, sizeof( info.szDeviceID ) / sizeof(info.szDeviceID[0]) );
        std::mbstowcs( info.szDeviceName, szDesc, sizeof( info.szDeviceName ) / sizeof( info.szDeviceName[0] ) );
        devices->push_back( info );
    }

    return TRUE;
}

DWORD  WindowsAudioDSound::DoRenderThread( )
{
    WAVEFORMATEX wfx;
    wfx.cbSize = 0;

        render_direct_sound_buf_->GetFormat( &wfx, );
    int frameSize = render_sample_rate_ / 100;
    int           framedelay = ( std::max )( 5UL, 1000 * frameSize / wfx.nAvgBytesPerSec );
    unsigned long dwBufferSize = ( std::max )( 2, ( MA_DELAY / framedelay ) ) * frameSize + frameSize;
    //fill silience
    {
        void*         plockedbuffer = NULL;
        unsigned long dwlockedsize = dwBufferSize;
        if ( FAILED( pDSB->Lock( 0, 0, &plockedbuffer, &dwlockedsize, NULL, NULL, DSBLOCK_ENTIREBUFFER ) ) )
            goto err;

        memset( (BYTE*)plockedbuffer, 0, dwlockedsize );
        pDSB->Unlock( plockedbuffer, dwlockedsize, NULL, 0 );
    }

    pDSB->SetCurrentPosition( 0 );
    if ( FAILED( pDSB->Play( 0, 0, DSBPLAY_LOOPING ) ) )
        goto err;

    m_openstatus = DEVICE_OPEN_SUCCEED;

    // play data
    {
        long          nNextOffset = m_frameSize;
        unsigned long dwCurDevPos = 0;
        unsigned long dwCurDataPos = 0;

        unsigned long filldelay_ttl = min_delay;
        bool          less_timepoint = GetTickCount();


        while ( m_opened )
        {
            if ( FAILED( hr = pDSB->GetCurrentPosition( &dwCurDevPos, &dwCurDataPos ) ) )
                break;

            LONG datasize = nNextOffset - dwCurDataPos;
            if ( datasize < 0 )
                datasize += dwBufferSize;

            if ( datasize <= min_delay )
            {
                if ( datasize < safe_delay )
                    filldelay_ttl = __min( dwBufferSize - m_frameSize, filldelay_ttl + m_frameSize );
                less_timepoint = GetTickCount();
            }
            else if ( GetTickCount() - less_timepoint > 4000 )
            {
                int oldttl = filldelay_ttl;

                filldelay_ttl = __max( min_delay, filldelay_ttl - m_frameSize );
                less_timepoint = GetTickCount();
            }


            if ( datasize <= filldelay_ttl )
            {
                void*         pbCaptureData = NULL;
                unsigned long dwLockedSize = 0;
                hr = pDSB->Lock( nNextOffset, m_frameSize, &pbCaptureData, &dwLockedSize, NULL, NULL, 0L );

                if ( FAILED( hr ) )
                    break;

                if ( DSBSTATUS_BUFFERLOST == hr )
                {
                    while ( m_opened )
                    {
                        hr = pDSB->Restore();
                        if ( hr == DSERR_BUFFERLOST )
                            Sleep( 5 );
                        else
                            break;
                    }

                    LOG( "buffer lost\n" );

                    hr = pDSB->Lock( nNextOffset, m_frameSize, &pbCaptureData, &dwLockedSize, NULL, NULL, 0L );

                    if ( FAILED( hr ) )
                        break;
                }

                LONG delayLength = nNextOffset - dwCurDevPos;
                if ( delayLength < 0 )
                    delayLength += dwBufferSize;

                bool breset = false;
                int nReaded = m_callback->ReadPlayoutSamples( pbCaptureData, dwLockedSize, delayLength, breset, m_userdata );

                if ( 0 == nReaded )
                    memset( pbCaptureData, 0, m_frameSize );

                pDSB->Unlock( pbCaptureData, dwLockedSize, NULL, NULL );

                nNextOffset += m_frameSize;
                if ( nNextOffset == dwBufferSize )
                    nNextOffset = 0;
            }
            else
            {
                Sleep( framedelay / 2 );
            }
        }

    }
}
