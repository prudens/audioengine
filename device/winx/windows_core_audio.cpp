#include "windows_core_audio.h"
#include "functiondiscoverykeys.h"           // PKEY_Device_FriendlyName
#include <strsafe.h>
#include <cassert>

namespace audio_engine
{
#define PARTID_MASK 0x0000ffff //HQH 增加一行



WindowsCoreAudio::WindowsCoreAudio()
{
    render_samples_ready_evnet_ = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    capture_samples_ready_event_ = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    shutdown_render_event_ = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    shutdown_capture_event_ = ::CreateEvent( NULL, TRUE, FALSE, NULL );
    render_started_event_ = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    capture_start_event_ = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    HRESULT hr = CoCreateInstance( CLSID_CWMAudioAEC,
                                   NULL,
                                   CLSCTX_INPROC_SERVER,
                                   IID_IMediaObject,
                                   reinterpret_cast<void**>( &dmo_ ) );
    if ( FAILED( hr ) || !dmo_ )
    {
        dmo_is_available_ = false; // audio effect such as aec ns etc.
    }

    hr = EnumDevice( eCapture, capture_device_list_ );
    hr = EnumDevice( eRender, render_device_list_ );
    int16_t size = (int16_t)capture_device_list_.size();
    for ( int16_t i = 0; i < size; i++ )
    {
        if ( capture_device_list_[i].bDefaultDevice )
        {
            capture_device_index_ = i;
            break;
        }
    }
    size = (int16_t)render_device_list_.size();
    for ( int16_t i = 0; i < size; i++ )
    {
        if ( render_device_list_[i].bDefaultDevice )
        {
            render_device_index_ = i;
            break;
        }
    }

    dmo_is_available_ = false;
}

WindowsCoreAudio::~WindowsCoreAudio()
{
    Terminate();
    RELEASE_HANDLE( render_samples_ready_evnet_ );
    RELEASE_HANDLE( capture_samples_ready_event_ );
    RELEASE_HANDLE( shutdown_render_event_ );
    RELEASE_HANDLE( shutdown_capture_event_ );
    RELEASE_HANDLE( render_started_event_ );
    RELEASE_HANDLE( capture_start_event_ );

    dmo_.Release();
}

void WindowsCoreAudio::Release()
{
    delete this;
}
bool WindowsCoreAudio::Initialize()
{
    {
        lock_guard lg( audio_lock_ );
        if ( initialize_ )
        {
            return true;
        }
        DeviceBindTo( eCapture, capture_device_index_, &audio_client_in_, nullptr, nullptr );
        DeviceBindTo( eRender, render_device_index_, &audio_client_out_, nullptr, nullptr );
        if ( !audio_client_in_ || !audio_client_in_ )
        {
            return false;
        }
    }
    initialize_ = true;
    for ( uint32_t sample_rate : {48000,44100,32000,16000,8000} )
    {
        for ( auto channel : {2,1} )
        {
            if ( IsRecordingFormatSupported( sample_rate, (uint16_t)channel ) )
            {
                capture_sample_rate_ = sample_rate;
                capture_channel_ = (int16_t)channel;
                break;
            }
        }
        if (capture_channel_ != 0 )
        {
            break;
        }
    }

    for ( uint32_t sample_rate : { 48000, 44100, 32000, 16000, 8000 } )
    {
        for ( auto channel : { 2, 1 } )
        {
            if ( IsPlayoutFormatSupported( sample_rate, (uint16_t)channel ) )
            {
                render_sample_rate_ = sample_rate;
                render_channel_ = (int16_t)channel;
                break;
            }
        }
        if (render_channel_ != 0)
        {
            break;
        }
    }


    return true;
}

void WindowsCoreAudio::Terminate()
{

    if (!initialize_)
    {
        return;
    }
    StopRecording();
    StopPlayout();
    lock_guard lg( audio_lock_ );
    audio_client_in_.Release();
    audio_client_out_.Release();
    capture_device_index_ = 0;
    render_device_index_ = 0;
    initialize_ = false;
    set_effect_.reset();
}

size_t WindowsCoreAudio::GetRecordingDeviceNum() const
{
    return capture_device_list_.size();
}

size_t WindowsCoreAudio::GetPlayoutDeviceNum() const
{
    return render_device_list_.size();
}

bool WindowsCoreAudio::GetPlayoutDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    lock_guard lg( audio_lock_ );

    if ( index >= (int16_t)render_device_list_.size() )
    {
        return false;
    }
    if ( index  < 0)
    {
        index = render_device_index_;
    }

    StringCchCopyW( name, kAdmMaxDeviceNameSize - 1, render_device_list_[index].szDeviceName );
    StringCchCopyW( guid, kAdmMaxGuidSize - 1, render_device_list_[index].szDeviceID );
    return true;
}


bool WindowsCoreAudio::GetRecordingDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    lock_guard lg( audio_lock_ );

    if ( index <= (int16_t)capture_device_list_.size() )
    {
        return false;
    }
    if ( index < 0 )
    {
        index = capture_device_index_;
    }
    StringCchCopyW( name, kAdmMaxDeviceNameSize - 1, capture_device_list_[index].szDeviceName );
    StringCchCopyW( guid, kAdmMaxGuidSize - 1, capture_device_list_[index].szDeviceID );
    return true;
}


bool WindowsCoreAudio::SetPlayoutDevice( int16_t index )
{
    lock_guard lg( audio_lock_ );
    if (initialize_)
    {
        return false;
    }
    if ( index <= (int16_t)render_device_list_.size() )
    {
        return false;
    }

    render_device_index_ = index;
    return true;
}

bool WindowsCoreAudio::SetRecordingDevice( int16_t index )
{
    lock_guard lg( audio_lock_ );
    if ( initialize_ )
    {
        return false;
    }
    if ( index <= (int16_t)capture_device_list_.size() )
    {
        return false;
    }

    capture_device_index_ = index;
    return true;
}

bool WindowsCoreAudio::IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    lock_guard lg( audio_lock_ );
    if ( !initialize_ )
    {
        return false;
    }

    return IsFormatSupported( audio_client_in_, nSampleRate, nChannels );
}

bool WindowsCoreAudio::IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    lock_guard lg( audio_lock_ );
    if ( !initialize_ )
    {
        return false;
    }
    return IsFormatSupported( audio_client_out_, nSampleRate, nChannels );
}

bool WindowsCoreAudio::SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    lock_guard lg( audio_lock_ );
    if ( !initialize_ )
    {
        return false;
    }

    if ( IsFormatSupported(audio_client_in_, nSampleRate, nChannels) )
    {
        capture_sample_rate_ = nSampleRate;
        capture_channel_ = static_cast<uint8_t>( nChannels );
        return true;
    }
    return false;

}

bool WindowsCoreAudio::SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    lock_guard lg( audio_lock_ );
    if (!initialize_)
    {
        return false;
    }
    if ( IsFormatSupported( audio_client_out_, nSampleRate, nChannels ) )
    {
        render_sample_rate_ = nSampleRate;
        render_channel_ = static_cast<uint8_t>( nChannels);
        return true;
    }
    return false;

}

bool WindowsCoreAudio::GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    lock_guard lg( audio_lock_ );
    if (!initialize_)
    {
        return false;
    }
    if (capture_sample_rate_ == 0 || capture_channel_ == 0)
    {
        return false;
    }
    nSampleRate = capture_sample_rate_;
    nChannels = capture_channel_;
    return true;

}

bool WindowsCoreAudio::GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    lock_guard lg( audio_lock_ );
    if ( !initialize_ )
    {
        return false;
    }

    if ( render_sample_rate_ == 0 || render_channel_ == 0 )
    {
        return false;
    }

    nSampleRate = render_sample_rate_;
    nChannels = render_channel_;
    return true;

}

bool WindowsCoreAudio::InitPlayout()
{
    lock_guard lg( audio_lock_ );
    if ( !initialize_ )
    {
        return false;
    }


    if ( use_dmo_ )
    {
        // Ensure the correct render device is configured in case
        // InitRecording() was called before InitPlayout().
        if ( FAILED( SetDMOProperties() ) )
        {
            return false;
        }
    }


    // Create a rendering stream.
    //
    // ****************************************************************************
    // For a shared-mode stream that uses event-driven buffering, the caller must
    // set both hnsPeriodicity and hnsBufferDuration to 0. The Initialize method
    // determines how large a buffer to allocate based on the scheduling period
    // of the audio engine. Although the client's buffer processing thread is
    // event driven, the basic buffer management process, as described previously,
    // is unaltered.
    // Each time the thread awakens, it should call IAudioClient::GetCurrentPadding
    // to determine how much data to write to a rendering buffer or read from a capture
    // buffer. In contrast to the two buffers that the Initialize method allocates
    // for an exclusive-mode stream that uses event-driven buffering, a shared-mode
    // stream requires a single buffer.
    // ****************************************************************************
    //
    REFERENCE_TIME hnsBufferDuration = 0;  // ask for minimum buffer size (default)
    if ( render_sample_rate_ == 44100 )
    {
        // Ask for a larger buffer size (30ms) when using 44.1kHz as render rate.
        // There seems to be a larger risk of underruns for 44.1 compared
        // with the default rate (48kHz). When using default, we set the requested
        // buffer duration to 0, which sets the buffer to the minimum size
        // required by the engine thread. The actual buffer size can then be
        // read by GetBufferSize() and it is 20ms on most machines.
        hnsBufferDuration = 30 * 10000;
    }
    HRESULT hr = S_OK;
    audio_client_out_.Release();

    DeviceBindTo( eRender, render_device_index_, &audio_client_out_, nullptr, nullptr );
    if ( render_sample_rate_ == 0 || render_channel_ == 0 )
    {
        WAVEFORMATEX* pWfxOut = NULL;
        hr = audio_client_out_->GetMixFormat( &pWfxOut );
        render_sample_rate_ = pWfxOut->nSamplesPerSec;
        render_channel_ = pWfxOut->nChannels;
        ::CoTaskMemFree( pWfxOut );
    }


    WAVEFORMATEX Wfx;
    Wfx.wFormatTag = WAVE_FORMAT_PCM;
    Wfx.wBitsPerSample = 16;
    Wfx.cbSize = 0;
    Wfx.nChannels = render_channel_;
    Wfx.nSamplesPerSec = render_sample_rate_;
    Wfx.nBlockAlign = Wfx.nChannels * Wfx.wBitsPerSample / 8;
    Wfx.nAvgBytesPerSec = Wfx.nSamplesPerSec * Wfx.nBlockAlign;

    hr = audio_client_out_->Initialize(
        AUDCLNT_SHAREMODE_SHARED,             // share Audio Engine with other applications
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,    // processing of the audio buffer by the client will be event driven
        hnsBufferDuration,                    // requested buffer capacity as a time value (in 100-nanosecond units)
        0,                                    // periodicity
        &Wfx,                                 // selected wave format
        NULL );                                // session GUID

    IF_FAILED_EXIT( hr );
    

    hr = audio_client_out_->SetEventHandle(
        render_samples_ready_evnet_ );
    IF_FAILED_EXIT( hr );

    // Get an IAudioRenderClient interface.
    render_client_.Release();// 有可能上次没释放成功
    hr = audio_client_out_->GetService(
        __uuidof( IAudioRenderClient ),
        (void**)&render_client_ );
    IF_FAILED_EXIT( hr );

    return true;

Exit:
    render_client_.Release();
    return false;

}

bool WindowsCoreAudio::InitRecording()
{
    lock_guard lg( audio_lock_ );
    if ( !initialize_ )
    {
        return false;
    }

    if ( use_dmo_ )
    {
        return InitRecordingDMO();
    }
    else
    {
        return SUCCEEDED( InitRecordingMedia() );
    }
}

bool WindowsCoreAudio::StartPlayout()
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

    if ( !InitPlayout() )
    {
        return false;
    }

    {
        std::lock_guard<std::mutex> lg(audio_lock_);
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

    DWORD ret = WaitForSingleObject( render_started_event_, 1000 );
    if ( ret != WAIT_OBJECT_0 )
    {
        return false;
    }

    playing_ = true;

    return true;
}

bool WindowsCoreAudio::StopPlayout()
{
    if ( !initialize_ )
    {
        return true;
    }
    if (!playing_)
    {
        return true;
    }
    {
        std::lock_guard<std::mutex> lg( audio_lock_ );

        if ( playout_thread_handle_ == NULL )
        {
            printf("no rendering stream is active => close down WASAPI only" );
            render_client_.Release();
            playing_ = false;
            return true;
        }

        printf( "closing down the webrtc_core_audio_render_thread..." );
        SetEvent( shutdown_render_event_ );
    }  // critScoped

    DWORD ret = WaitForSingleObject( playout_thread_handle_, 2000 );
    if ( ret != WAIT_OBJECT_0 )
    {
        // the thread did not stop as it should
        printf("failed to close down webrtc_core_audio_render_thread" );
        CloseHandle( playout_thread_handle_ );
        playout_thread_handle_ = NULL;
        playing_ = false;
        return false;
    }

    {
        std::lock_guard<std::mutex> lg( audio_lock_ );
        printf("windows_core_audio_render_thread is now closed" );

        // to reset this event manually at each time we finish with it,
        // in case that the render thread has exited before StopPlayout(),
        // this event might be caught by the new render thread within same VoE instance.
        ResetEvent( shutdown_render_event_ );

        render_client_.Release();
        playing_ = false;

        CloseHandle( playout_thread_handle_ );
        playout_thread_handle_ = NULL;

        if ( use_dmo_ && recording_ )
        {
            // The DMO won't provide us captured output data unless we
            // give it render data to process.
            //
            // We still permit the playout to shutdown, and trace a warning.
            // Otherwise, VoE can get into a state which will never permit
            // playout to stop properly.
           printf( "Recording should be stopped before playout when using the "
                          "built-in AEC" );
        }
    }  // critScoped
    return true;
}

bool WindowsCoreAudio::Playing() const
{
    return playing_;

}

bool WindowsCoreAudio::StartRecording()
{
    if (!initialize_)
    {
        return false;
    }

    if ( recording_thread_handle_ != NULL )
    {
        return true;
    }

    if ( recording_ )
    {
        return true;
    }

    if ( !InitRecording() )
    {
        return false;
    }

    {
        std::lock_guard<std::mutex> lg(audio_lock_);

        // Create thread which will drive the capturing
        LPTHREAD_START_ROUTINE lpStartAddress = WSAPICaptureThread;
        if ( use_dmo_ )
        {
            // Redirect to the DMO polling method.
            lpStartAddress = WSAPICaptureThreadPollDMO;

            if ( !playing_ )
            {
                // The DMO won't provide us captured output data unless we
                // give it render data to process.
                printf( "Playout must be started before recording when using the built-in AEC" );
                return false;
            }
        }

        assert( recording_thread_handle_ == NULL );
        recording_thread_handle_ = CreateThread( NULL,
                                    0,
                                    lpStartAddress,
                                    this,
                                    0,
                                    NULL );
        if ( recording_thread_handle_ == NULL )
        {
            printf( "failed to create the recording thread" );
            return false;
        }

        // Set thread priority to highest possible
        ::SetThreadPriority( recording_thread_handle_, THREAD_PRIORITY_TIME_CRITICAL );


    }  // critScoped

    DWORD ret = WaitForSingleObject( capture_start_event_, 1000 );
    if ( ret != WAIT_OBJECT_0 )
    {
        printf( "capturing did not start up properly" );
        return false;
    }
    printf( "capture audio stream has now started..." );


    recording_ = true;
    return true;
}

bool WindowsCoreAudio::StopRecording()
{
    bool err = true;

    if ( !initialize_ )
    {
        return 0;
    }

    
    {
        lock_guard lg( audio_lock_ );
        if ( recording_thread_handle_ == NULL )
        {
            printf( "no capturing stream is active => close down WASAPI only" );
            capture_client_.Release();

            recording_ = false;
            return 0;
        }

        // Stop the driving thread...
        printf( "closing down the webrtc_core_audio_capture_thread..." );
        // Manual-reset event; it will remain signalled to stop all capture threads.
        SetEvent( shutdown_capture_event_ );
    }

    DWORD ret = WaitForSingleObject( recording_thread_handle_, 2000 );
    if ( ret != WAIT_OBJECT_0 )
    {
        printf( "failed to close down webrtc_core_audio_capture_thread" );
        err = false;
    }
    else
    {
        printf("webrtc_core_audio_capture_thread is now closed" );
    }

    lock_guard lg( audio_lock_ );

    ResetEvent( shutdown_capture_event_ ); // Must be manually reset.
    // Ensure that the thread has released these interfaces properly.


    recording_ = false;

    // These will create thread leaks in the result of an error,
    // but we can at least resume the call.
    CloseHandle( recording_thread_handle_ );
    recording_thread_handle_ = NULL;

    if ( use_dmo_ )
    {
        assert( dmo_ );
        // This is necessary. Otherwise the DMO can generate garbage render
        // audio even after rendering has stopped.
        HRESULT hr = dmo_->FreeStreamingResources();
        if ( FAILED( hr ) )
        {
            err = false;
        }
    }

    return err;

}

bool WindowsCoreAudio::Recording() const
{
    return recording_;
}

void WindowsCoreAudio::SetAudioBufferCallback( AudioBufferProc* pCallback )
{
    audio_buffer_proc_ = pCallback;
}

AudioBufferProc* WindowsCoreAudio::GetAudioBufferCallback()const
{
    return audio_buffer_proc_;
}

bool WindowsCoreAudio::SetPropertie( AudioPropertyID id, void*value )
{
    switch ( id )
    {
    case ID_ENABLE_AEC:
    case ID_ENBALE_AGC:
    case ID_ENBALE_NS:
    case ID_ENBALE_VAD:
    {
        if ( !dmo_is_available_ )
        {
            return false;
        }
        bool var = *(int32_t*)value != 0;
        set_effect_.set( ID_ENABLE_AEC, var );     
        {
            set_effect_.set( id, var );
        }
    }
    break;
    }

    if (set_effect_.to_ulong() & (ID_ENABLE_AEC | ID_ENBALE_AGC | ID_ENBALE_NS | ID_ENBALE_VAD) )
    {
        use_dmo_ = true;
    }
    else
    {
        use_dmo_ = false;
    }
    return true;
}

bool WindowsCoreAudio::GetProperty( AudioPropertyID id, void*value )
{
    if ( !use_dmo_ )
    {
        return false;
    }
    *(int32_t*)value = set_effect_.test( id ) ? 1 : 0;
    return true;
}

bool WindowsCoreAudio::IsFormatSupported( CComPtr<IAudioClient> audioClient, DWORD nSampleRate, WORD nChannels )
{
    if ( !audioClient )
    {
        return false;
    }
    HRESULT hr = S_OK;
    WAVEFORMATEX Wfx = WAVEFORMATEX();
    WAVEFORMATEX* pWfxClosestMatch = NULL;
    // Set wave format
    Wfx.wFormatTag = WAVE_FORMAT_PCM;
    Wfx.wBitsPerSample = 16;
    Wfx.cbSize = 0;
    Wfx.nChannels = nChannels;
    Wfx.nSamplesPerSec = nSampleRate;
    Wfx.nBlockAlign = Wfx.nChannels * Wfx.wBitsPerSample / 8;
    Wfx.nAvgBytesPerSec = Wfx.nSamplesPerSec * Wfx.nBlockAlign;
    hr = audioClient->IsFormatSupported(
        AUDCLNT_SHAREMODE_SHARED,
        &Wfx,
        &pWfxClosestMatch );

    CoTaskMemFree( pWfxClosestMatch );
    return hr == S_OK;
}

bool WindowsCoreAudio::SetDMOProperties()
{
    HRESULT hr = S_OK;
    assert( dmo_ != NULL );

    CComPtr<IPropertyStore> ps;
    hr = dmo_->QueryInterface( IID_IPropertyStore,
                                  reinterpret_cast<void**>( &ps ) );
    IF_FAILED_EXIT( hr );

    // Set the AEC system mode.
    // SINGLE_CHANNEL_AEC - AEC processing only.
    bool var = set_effect_.test( ID_ENABLE_AEC );
    hr = SetVtI4Property( ps,
                          MFPKEY_WMAAECMA_SYSTEM_MODE,
                          var ? SINGLE_CHANNEL_AEC: SINGLE_CHANNEL_NSAGC );
    IF_FAILED_EXIT( hr );


    // Set the AEC source mode.
    // VARIANT_TRUE - Source mode (we poll the AEC for captured data).
    hr = SetBoolProperty( ps,
                          MFPKEY_WMAAECMA_DMO_SOURCE_MODE,
                          VARIANT_TRUE );
    IF_FAILED_EXIT( hr );

    // Enable the feature mode.
    // This lets us override all the default processing settings below.

    hr = SetBoolProperty( ps,
                          MFPKEY_WMAAECMA_FEATURE_MODE,
                          VARIANT_TRUE );
    IF_FAILED_EXIT( hr );
    var = set_effect_.test( ID_ENBALE_AGC );
    // Disable analog AGC (default enabled).
    hr = SetBoolProperty( ps,
                          MFPKEY_WMAAECMA_MIC_GAIN_BOUNDER,
                          var ? VARIANT_TRUE : VARIANT_FALSE );
    IF_FAILED_EXIT( hr );

    // Disable noise suppression (default enabled).
    // 0 - Disabled, 1 - Enabled
    var = set_effect_.test( ID_ENBALE_NS );
    hr = SetVtI4Property( ps,
                          MFPKEY_WMAAECMA_FEATR_NS,
                          var ? 1 : 0 );
    IF_FAILED_EXIT( hr );

    var = set_effect_.test( ID_ENBALE_VAD );
    hr = SetVtI4Property( ps,
                          MFPKEY_WMAAECMA_FEATR_VAD,
                          var ? 1 : 0 );
    IF_FAILED_EXIT( hr );

    // Relevant parameters to leave at default settings:
    // MFPKEY_WMAAECMA_FEATR_AGC - Digital AGC (disabled).
    // MFPKEY_WMAAECMA_FEATR_CENTER_CLIP - AEC center clipping (enabled).
    // MFPKEY_WMAAECMA_FEATR_ECHO_LENGTH - Filter length (256 ms).
    //   TODO(andrew): investigate decresing the length to 128 ms.
    // MFPKEY_WMAAECMA_FEATR_FRAME_SIZE - Frame size (0).
    //   0 is automatic; defaults to 160 samples (or 10 ms frames at the
    //   selected 16 kHz) as long as mic array processing is disabled.
    // MFPKEY_WMAAECMA_FEATR_NOISE_FILL - Comfort noise (enabled).
    // MFPKEY_WMAAECMA_FEATR_VAD - VAD (disabled).

    // Set the devices selected by VoE. If using a default device, we need to
    // search for the device index.

    DWORD devIndex = static_cast<uint32_t>( render_device_index_ << 16 ) +
        static_cast<uint32_t>( 0x0000ffff & capture_device_index_ );
    hr = SetVtI4Property( ps,
                          MFPKEY_WMAAECMA_DEVICE_INDEXES,
                          devIndex );

Exit:
    return  SUCCEEDED( hr );
}

bool WindowsCoreAudio::InitRecordingMedia()
{
    if ( initialize_ )
    {
        return true;
    }

    audio_client_in_.Release();
    DeviceBindTo( eCapture, capture_device_index_, &audio_client_in_, nullptr, nullptr );
    HRESULT hr = S_OK;
    if ( capture_sample_rate_ == 0 || capture_channel_ == 0 )
    {
        WAVEFORMATEX* pWfxIn = NULL;
        hr = audio_client_out_->GetMixFormat( &pWfxIn );
        IF_FAILED_EXIT( hr );
        capture_sample_rate_ = pWfxIn->nSamplesPerSec;
        capture_channel_ = pWfxIn->nChannels;
        ::CoTaskMemFree( pWfxIn );
    }

    // Set wave format
    WAVEFORMATEX Wfx;
    Wfx.wFormatTag = WAVE_FORMAT_PCM;
    Wfx.wBitsPerSample = 16;
    Wfx.cbSize = 0;
    Wfx.nChannels = capture_channel_;
    Wfx.nSamplesPerSec = capture_sample_rate_;
    Wfx.nBlockAlign = Wfx.nChannels * Wfx.wBitsPerSample / 8;
    Wfx.nAvgBytesPerSec = Wfx.nSamplesPerSec * Wfx.nBlockAlign;

    // Create a capturing stream.

    hr = audio_client_in_->Initialize(
        AUDCLNT_SHAREMODE_SHARED,             // share Audio Engine with other applications
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK |   // processing of the audio buffer by the client will be event driven
        AUDCLNT_STREAMFLAGS_NOPERSIST,        // volume and mute settings for an audio session will not persist across system restarts
        0,                                    // required for event-driven shared mode
        0,                                    // periodicity
        &Wfx,                                 // selected wave format
        NULL );                                // session GUID
    IF_FAILED_EXIT( hr );

    // Set the event handle that the system signals when an audio buffer is ready
    // to be processed by the client.
    hr = audio_client_in_->SetEventHandle(
        capture_samples_ready_event_ );
    IF_FAILED_EXIT( hr );

    // Get an IAudioCaptureClient interface.
    capture_client_.Release();// 防止上次没有释放
    hr = audio_client_in_->GetService(
        __uuidof( IAudioCaptureClient ),
        (void**)&capture_client_ );
    IF_FAILED_EXIT( hr );
    return true;
Exit:
    return false;
}


// Capture initialization when the built-in AEC DirectX Media Object (DMO) is
// used. Called from InitRecording(), most of which is skipped over. The DMO
// handles device initialization itself.
// Reference: http://msdn.microsoft.com/en-us/library/ff819492(v=vs.85).aspx
bool WindowsCoreAudio::InitRecordingDMO()
{
    if ( !use_dmo_ || !dmo_ )
    {
        return false;
    }
    if ( !initialize_ )
    {
        return false;
    }

    if ( !SetDMOProperties() )
    {
        return false;
    }

    DMO_MEDIA_TYPE mt = { 0 };
    HRESULT hr = MoInitMediaType( &mt, sizeof( WAVEFORMATEX ) );
    if ( FAILED( hr ) )
    {
        MoFreeMediaType( &mt );
        return false;
    }
    mt.majortype = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;
    mt.formattype = FORMAT_WaveFormatEx;

    // Supported formats
    // nChannels: 1 (in AEC-only mode)
    // nSamplesPerSec: 8000, 11025, 16000, 22050
    // wBitsPerSample: 16
    WAVEFORMATEX* ptrWav = reinterpret_cast<WAVEFORMATEX*>( mt.pbFormat );
    ptrWav->wFormatTag = WAVE_FORMAT_PCM;
    ptrWav->nChannels = 1;
    // 16000 is the highest we can support with our resampler.
    ptrWav->nSamplesPerSec = 16000;
    ptrWav->nAvgBytesPerSec = 32000;
    ptrWav->nBlockAlign = 2;
    ptrWav->wBitsPerSample = 16;
    ptrWav->cbSize = 0;

    // Set the VoE format equal to the AEC output format.
    if (capture_channel_ != ptrWav->nChannels &&
         capture_sample_rate_ != ptrWav->nSamplesPerSec )
    {
        if ( audio_buffer_proc_ )
        {
            audio_buffer_proc_->ErrorOccurred( AE_RECORD_FORMAT_CHANGE );
        }
    }
    capture_sample_rate_ = ptrWav->nSamplesPerSec;
    capture_channel_ = ptrWav->nChannels;
    media_buffer_ = nullptr;
    hr = CMediaBuffer::Create( ptrWav->nBlockAlign * ptrWav->nSamplesPerSec/100, (IMediaBuffer**)&media_buffer_ );
    if ( FAILED( hr ) )
    {
        return false;
    }
    // Set the DMO output format parameters.
    hr = dmo_->SetOutputType( 0, &mt, 0 );
    ::MoFreeMediaType( &mt );
    if ( FAILED( hr ) )
    {
        return false;
    }

    // Optional, but if called, must be after media types are set.
    hr = dmo_->AllocateStreamingResources();
    if ( FAILED( hr ) )
    {
        return false;
    }


    return true;
}

DWORD WindowsCoreAudio::DoRenderThread()
{
    bool keepPlaying = true;
    HANDLE waitArray[2] = { shutdown_render_event_, render_samples_ready_evnet_ };
    HRESULT hr = S_OK;


    // Initialize COM as MTA in this thread.
    ScopedCOMInitializer comInit( ScopedCOMInitializer::kMTA );
    if ( !comInit.succeeded() )
    {
        return 1;
    }

    HANDLE hMmTask = SetThreadPriority( "windows_core_audio_render_thread" );

    audio_lock_.lock();

    // Get size of rendering buffer (length is expressed as the number of audio frames the buffer can hold).
    // This value is fixed during the rendering session.
    //
    UINT32 bufferLength = 0;
    hr = audio_client_out_->GetBufferSize( &bufferLength );
    IF_FAILED_JUMP( hr,Exit );
   
    const size_t playBlockSize = render_sample_rate_ / 100;
    const size_t playFrameSize = 2 * render_channel_;
    const double endpointBufferSizeMS = 10.0 * ( (double)bufferLength / (double)playBlockSize );

    // Before starting the stream, fill the rendering buffer with silence.
    //
    BYTE *pData = NULL;
    hr = render_client_->GetBuffer( bufferLength, &pData );
    IF_FAILED_JUMP( hr,Exit );

    hr = render_client_->ReleaseBuffer( bufferLength, AUDCLNT_BUFFERFLAGS_SILENT );
    IF_FAILED_JUMP( hr, Exit );

    // Start up the rendering audio stream.
    hr = audio_client_out_->Start();
    IF_FAILED_JUMP( hr, Exit );

    audio_lock_.unlock();

    // Set event which will ensure that the calling thread modifies the playing state to true.
    SetEvent( render_started_event_ );

    // >> ------------------ THREAD LOOP ------------------
    while ( keepPlaying )
    {
        // Wait for a render notification event or a shutdown event
        DWORD waitResult = WaitForMultipleObjects( 2, waitArray, FALSE, 500 );
        switch ( waitResult )
        {
        case WAIT_OBJECT_0 + 0:     // _hShutdownRenderEvent
            keepPlaying = false;
            break;
        case WAIT_OBJECT_0 + 1:     // _hRenderSamplesReadyEvent
            break;
        case WAIT_TIMEOUT:          // timeout notification
            printf( "render event timed out after 0.5 seconds" );
            goto Exit;
        default:                    // unexpected error
            printf( "unknown wait termination on render side" );
            goto Exit;
        }

        while ( keepPlaying )
        {
            audio_lock_.lock();

            // Sanity check to ensure that essential states are not modified
            // during the unlocked period.
            if ( !render_client_ || !audio_client_out_ )
            {
                audio_lock_.unlock();
                printf( "output state has been modified during unlocked period\n" );
                goto Exit;
            }

            // Get the number of frames of padding (queued up to play) in the endpoint buffer.
            UINT32 padding = 0;
            hr = audio_client_out_->GetCurrentPadding( &padding );
            IF_FAILED_JUMP( hr,Exit );

            // Derive the amount of available space in the output buffer
            uint32_t framesAvailable = bufferLength - padding;
            // WEBRTC_TRACE(kTraceStream, kTraceAudioDevice, _id, "#avaliable audio frames = %u", framesAvailable);

            // Do we have 10 ms available in the render buffer?
            if ( framesAvailable < playBlockSize )
            {
                // Not enough space in render buffer to store next render packet.
                audio_lock_.unlock();
                break;
            }

            // Write n*10ms buffers to the render buffer
            const uint32_t n10msBuffers = ( framesAvailable / playBlockSize );
            for ( uint32_t n = 0; n < n10msBuffers; n++ )
            {
                // Get pointer (i.e., grab the buffer) to next space in the shared render buffer.
                hr = render_client_->GetBuffer( playBlockSize, &pData );
                IF_FAILED_JUMP( hr, Exit );

                if ( audio_buffer_proc_ )
                {
                    audio_lock_.unlock();
                    int32_t nSamples = audio_buffer_proc_->NeedMorePlayoutData( (int16_t*)pData, playBlockSize*playFrameSize );
                    nSamples /= playFrameSize;
                    // Request data to be played out (#bytes = _playBlockSize*_audioFrameSize)
                    
                    audio_lock_.lock();

                    if ( nSamples == -1 )
                    {
                        audio_lock_.unlock();
                        printf( "failed to read data from render client\n" );
                        goto Exit;
                    }

                    // Sanity check to ensure that essential states are not modified during the unlocked period
                    if ( !render_client_ || !audio_client_out_ )
                    {
                        audio_lock_.unlock();
                        printf( "output state has been modified during unlocked period\n" );
                        goto Exit;
                    }
                    if ( nSamples != static_cast<int32_t>( playBlockSize ) )
                    {
                        printf("nSamples(%d) != _playBlockSize(%d)\n", nSamples, playBlockSize );
                    }
                }

                DWORD dwFlags( 0 );
                hr = render_client_->ReleaseBuffer( playBlockSize, dwFlags );
                // See http://msdn.microsoft.com/en-us/library/dd316605(VS.85).aspx
                // for more details regarding AUDCLNT_E_DEVICE_INVALIDATED.
                IF_FAILED_JUMP( hr,Exit );
            }

            audio_lock_.unlock();
        }
    }

    // ------------------ THREAD LOOP ------------------ <<

    Sleep( static_cast<DWORD>( endpointBufferSizeMS + 0.5 ) );
    hr = audio_client_out_->Stop();

Exit:
    if ( FAILED( hr ) )
    {
        audio_client_out_->Stop();
        audio_lock_.unlock();
    }

    RevertThreadPriority( hMmTask );

    audio_lock_.lock();

    if ( keepPlaying )
    {
        if ( audio_client_out_ )
        {
            hr = audio_client_out_->Stop();
            audio_client_out_->Reset();
        }
        // Trigger callback from module process thread
        printf( "kPlayoutError message posted: rendering thread has ended pre-maturely" );
    }
    else
    {
        printf( "_Rendering thread is now terminated properly" );
    }

    audio_lock_.unlock();

    return (DWORD)hr;
}

DWORD WindowsCoreAudio::DoCaptureThreadPollDMO()
{
    bool keepRecording = true;
    printf("Start Recording Thread with dmo");
    // Initialize COM as MTA in this thread.
    ScopedCOMInitializer comInit( ScopedCOMInitializer::kMTA );
    if ( !comInit.succeeded() )
    {
        printf("failed to initialize COM in polling DMO thread" );
        return true;
    }

    HANDLE hMmTask = SetThreadPriority( "windows_core_audio_capture_thread" );

    // Set event which will ensure that the calling thread modifies the
    // recording state to true.
    SetEvent( capture_start_event_ );
    HRESULT hr = S_OK;
    // >> ---------------------------- THREAD LOOP ----------------------------
    while ( keepRecording )
    {
        // Poll the DMO every 5 ms.
        // (The same interval used in the Wave implementation.)
        DWORD waitResult = WaitForSingleObject( shutdown_capture_event_, 5 );
        switch ( waitResult )
        {
        case WAIT_OBJECT_0:         // _hShutdownCaptureEvent
            keepRecording = false;
            break;
        case WAIT_TIMEOUT:          // timeout notification
            break;
        default:                    // unexpected error
            printf( "Unknown wait termination on capture side" );
            hr = -1; // To signal an error callback.
            keepRecording = false;
            break;
        }

        while ( keepRecording )
        {
            std::unique_lock<std::mutex> ul(audio_lock_);
            DWORD dwStatus = 0;
            {
                DMO_OUTPUT_DATA_BUFFER dmoBuffer = { 0 };
                dmoBuffer.pBuffer = media_buffer_;
                dmoBuffer.pBuffer->AddRef();
                // Poll the DMO for AEC processed capture data. The DMO will
                // copy available data to |dmoBuffer|, and should only return
                // 10 ms frames. The value of |dwStatus| should be ignored.
                hr = dmo_->ProcessOutput( 0, 1, &dmoBuffer, &dwStatus );
                dmoBuffer.pBuffer->Release();
                dwStatus = dmoBuffer.dwStatus;
            }
            if ( FAILED( hr ) )
            {
                keepRecording = false;
                assert( false );
                break;
            }

            DWORD bytesProduced = 0;
            BYTE* data;
            // Get a pointer to the data buffer. This should be valid until
            // the next call to ProcessOutput.
            hr = media_buffer_->GetBufferAndLength( &data, &bytesProduced );
            if ( FAILED( hr ) )
            {
                keepRecording = false;
                assert( false );
                break;
            }

            // TODO(andrew): handle AGC.

            if ( bytesProduced > 0 )
            {
                const int kSamplesProduced = bytesProduced;
                // TODO(andrew): verify that this is always satisfied. It might
                // be that ProcessOutput will try to return more than 10 ms if
                // we fail to call it frequently enough.


                ul.unlock();  // Release lock while making the callback.
                if ( audio_buffer_proc_ )
                {
                    audio_buffer_proc_->RecordingDataIsAvailable( (int16_t*)data, kSamplesProduced );
                }
                ul.lock();
            }

            // Reset length to indicate buffer availability.
            hr = media_buffer_->SetLength( 0 );
            if ( FAILED( hr ) )
            {
                keepRecording = false;
                assert( false );
                break;
            }

            if ( !( dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE ) )
            {
                // The DMO cannot currently produce more data. This is the
                // normal case; otherwise it means the DMO had more than 10 ms
                // of data available and ProcessOutput should be called again.
                break;
            }
        }

    }
    // ---------------------------- THREAD LOOP ---------------------------- <<

    RevertThreadPriority( hMmTask );
    printf( "Stop Recording Thread with dmo" );

    return hr;
}



DWORD WindowsCoreAudio::DoCaptureThread()
{
    bool keepRecording = true;
    HANDLE waitArray[2] = { shutdown_capture_event_, capture_samples_ready_event_ };
    HRESULT hr = S_OK;

    BYTE* syncBuffer = NULL;
    UINT32 syncBufIndex = 0;

    // Initialize COM as MTA in this thread.
    ScopedCOMInitializer comInit( ScopedCOMInitializer::kMTA );
    if ( !comInit.succeeded() )
    {
        printf( "failed to initialize COM in capture thread\n" );
        return 1;
    }

    HANDLE hMmTask = SetThreadPriority("windows_core_audio_capture_thread");

    audio_lock_.lock();

    // Get size of capturing buffer (length is expressed as the number of audio frames the buffer can hold).
    // This value is fixed during the capturing session.
    //
    UINT32 bufferLength = 0;
    if ( !audio_client_in_ )
    {
        return 1;
    }
    hr = audio_client_in_->GetBufferSize( &bufferLength );
    IF_FAILED_JUMP( hr,Exit );

    // Allocate memory for sync buffer.
    // It is used for compensation between native 44.1 and internal 44.0 and
    // for cases when the capture buffer is larger than 10ms.
    //
    const UINT32 syncBufferSize = 2 * ( bufferLength * 2 * capture_channel_ );
    syncBuffer = new BYTE[syncBufferSize];
    if ( syncBuffer == NULL )
    {
        return (DWORD)E_POINTER;
    }
    printf( "[CAPT] size of sync buffer  : %u [bytes]\n", syncBufferSize );
    const size_t recBlockSize = capture_sample_rate_ / 100;
    const size_t recAudioFrameSize = 2 * capture_channel_;
    const double endpointBufferSizeMS = 10.0 * ( (double)bufferLength / (double)recAudioFrameSize );
    printf( "[CAPT] endpointBufferSizeMS : %3.2f\n", endpointBufferSizeMS );

    // Start up the capturing stream.
    //
    hr = audio_client_in_->Start();
    IF_FAILED_JUMP( hr,Exit );

    audio_lock_.unlock();

    // Set event which will ensure that the calling thread modifies the recording state to true.
    //
    SetEvent( capture_start_event_ );

    // >> ---------------------------- THREAD LOOP ----------------------------

    while ( keepRecording )
    {
        // Wait for a capture notification event or a shutdown event
        DWORD waitResult = WaitForMultipleObjects( 2, waitArray, FALSE, 500 );
        switch ( waitResult )
        {
        case WAIT_OBJECT_0 + 0:        // _hShutdownCaptureEvent
            keepRecording = false;
            break;
        case WAIT_OBJECT_0 + 1:        // _hCaptureSamplesReadyEvent
            break;
        case WAIT_TIMEOUT:            // timeout notification
            printf( "capture event timed out after 0.5 seconds\n" );
            goto Exit;
        default:                    // unexpected error
            printf( "unknown wait termination on capture side\n" );
            goto Exit;
        }

        while ( keepRecording )
        {
            BYTE *pData = 0;
            UINT32 framesAvailable = 0;
            DWORD flags = 0;
            UINT64 recTime = 0;
            UINT64 recPos = 0;

            audio_lock_.lock();

            // Sanity check to ensure that essential states are not modified
            // during the unlocked period.
            if ( !capture_client_  || !audio_client_in_ )
            {
                audio_lock_.unlock();
                printf("input state has been modified during unlocked period\n" );
                goto Exit;
            }

            //  Find out how much capture data is available
            //
            hr = capture_client_->GetBuffer( &pData,           // packet which is ready to be read by used
                                               &framesAvailable, // #frames in the captured packet (can be zero)
                                               &flags,           // support flags (check)
                                               &recPos,          // device position of first audio frame in data packet
                                               &recTime );        // value of performance counter at the time of recording the first audio frame

            if ( SUCCEEDED( hr ) )
            {
                if ( AUDCLNT_S_BUFFER_EMPTY == hr )
                {
                    // Buffer was empty => start waiting for a new capture notification event
                    audio_lock_.unlock();
                    break;
                }

                if ( flags & AUDCLNT_BUFFERFLAGS_SILENT )
                {
                    // Treat all of the data in the packet as silence and ignore the actual data values.
                    printf("AUDCLNT_BUFFERFLAGS_SILENT\n" );
                    pData = NULL;
                }

                assert( framesAvailable != 0 );

                if ( pData )
                {
                    CopyMemory( &syncBuffer[syncBufIndex*recAudioFrameSize], pData, framesAvailable*recAudioFrameSize );
                }
                else
                {
                    ZeroMemory( &syncBuffer[syncBufIndex*recAudioFrameSize], framesAvailable*recAudioFrameSize );
                }
                assert( syncBufferSize >= ( syncBufIndex*recAudioFrameSize ) + framesAvailable*recAudioFrameSize );

                // Release the capture buffer
                //
                hr = capture_client_->ReleaseBuffer( framesAvailable );
                IF_FAILED_JUMP( hr,Exit );

                syncBufIndex += framesAvailable;

                while ( syncBufIndex >= recBlockSize )
                {
                    audio_lock_.unlock();
                    if (audio_buffer_proc_)
                    {
                        audio_buffer_proc_->RecordingDataIsAvailable( (const int16_t*)syncBuffer, recBlockSize*recAudioFrameSize );
                    }
                    audio_lock_.lock();
                    // Sanity check to ensure that essential states are not modified during the unlocked period
                    if ( !capture_client_ || !audio_client_in_ )
                    {
                        audio_lock_.unlock();
                        printf( "input state has been modified during unlocked period\n" );
                        goto Exit;
                    }

                    // store remaining data which was not able to deliver as 10ms segment
                    MoveMemory( &syncBuffer[0], &syncBuffer[recBlockSize*recAudioFrameSize], ( syncBufIndex - recBlockSize )*recAudioFrameSize );
                    syncBufIndex -= recBlockSize;
                }
            }
            else
            {
                // If GetBuffer returns AUDCLNT_E_BUFFER_ERROR, the thread consuming the audio samples
                // must wait for the next processing pass. The client might benefit from keeping a count
                // of the failed GetBuffer calls. If GetBuffer returns this error repeatedly, the client
                // can start a new processing loop after shutting down the current client by calling
                // IAudioClient::Stop, IAudioClient::Reset, and releasing the audio client.
                printf( "IAudioCaptureClient::GetBuffer returned AUDCLNT_E_BUFFER_ERROR, hr = 0x%08X\n", hr );
                goto Exit;
            }

            audio_lock_.unlock();
        }
    }

    // ---------------------------- THREAD LOOP ---------------------------- <<

    if ( audio_client_in_ )
    {
        hr = audio_client_in_->Stop();
    }

Exit:
    if ( FAILED( hr ) )
    {
        audio_client_in_->Stop();
        audio_lock_.unlock();
    }

    RevertThreadPriority(hMmTask);

    audio_lock_.lock();

    if ( keepRecording )
    {
        if ( audio_client_in_ )
        {
            hr = audio_client_in_->Stop();
            audio_client_in_->Reset();
        }

        // Trigger callback from module process thread
        printf("kRecordingError message posted: capturing thread has ended pre-maturely\n" );
    }
    else
    {
        printf( "_Capturing thread is now terminated properly\n" );
    }

    capture_client_.Release();

    audio_lock_.unlock();

    if ( syncBuffer )
    {
        delete[] syncBuffer;
    }

    return (DWORD)hr;
}



DWORD WINAPI WindowsCoreAudio::WSAPIRenderThread( LPVOID context )
{
    return reinterpret_cast<WindowsCoreAudio*>( context )->
        DoRenderThread();
}

DWORD WINAPI WindowsCoreAudio::WSAPICaptureThreadPollDMO( LPVOID context )
{
    return reinterpret_cast<WindowsCoreAudio*>( context )->
        DoCaptureThreadPollDMO();
}


DWORD WINAPI WindowsCoreAudio::WSAPICaptureThread( LPVOID context )
{
    return reinterpret_cast<WindowsCoreAudio*>( context )->
        DoCaptureThread();
}

}