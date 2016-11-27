#include "windows_core_audio.h"
#include "functiondiscoverykeys.h"           // PKEY_Device_FriendlyName
#include <strsafe.h>
#include <cassert>

#define PARTID_MASK 0x0000ffff //HQH 增加一行


AudioDevice* AudioDevice::Create()
{
    return new WindowsCoreAudio();
}

WindowsCoreAudio::WindowsCoreAudio()
{
    m_hRenderSamplesReadyEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hCaptureSamplesReadyEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hShutdownRenderEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hShutdownCaptureEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hRenderStartedEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hCaptureStartedEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    HRESULT hr = CoCreateInstance( CLSID_CWMAudioAEC,
                                   NULL,
                                   CLSCTX_INPROC_SERVER,
                                   IID_IMediaObject,
                                   reinterpret_cast<void**>( &m_spDmo ) );
    if ( FAILED( hr ) || !m_spDmo )
    {
        m_DMOIsAvailble = false; // audio effect such as aec ns etc.
    }

    hr = EnumDevice( eCapture, m_CaptureDevices );
    hr = EnumDevice( eRender, m_RenderDevices );
    int16_t size = (int16_t)m_CaptureDevices.size();
    for ( int16_t i = 0; i < size; i++ )
    {
        if ( m_CaptureDevices[i].bDefaultDevice )
        {
            m_recDevIndex = i;
            break;
        }
    }
    size = (int16_t)m_RenderDevices.size();
    for ( int16_t i = 0; i < size; i++ )
    {
        if ( m_RenderDevices[i].bDefaultDevice )
        {
            m_plyDevIndex = i;
            break;
        }
    }
}

WindowsCoreAudio::~WindowsCoreAudio()
{
    Terminate();
    RELEASE_HANDLE( m_hRenderSamplesReadyEvent );
    RELEASE_HANDLE( m_hCaptureSamplesReadyEvent );
    RELEASE_HANDLE( m_hShutdownRenderEvent );
    RELEASE_HANDLE( m_hShutdownCaptureEvent );
    RELEASE_HANDLE( m_hRenderStartedEvent );
    RELEASE_HANDLE( m_hCaptureStartedEvent );

    m_spDmo.Release();
}

void WindowsCoreAudio::Release()
{
    delete this;
}
bool WindowsCoreAudio::Initialize()
{
    lock_guard lg(m_audiolock);
    if ( m_bInitialize )
    {
        return true;
    }
    DeviceBindTo( eCapture, m_recDevIndex, &m_spClientIn, nullptr, nullptr );
    DeviceBindTo( eRender, m_plyDevIndex, &m_spClientOut, nullptr, nullptr );
    if (!m_spClientIn || !m_spClientOut )
    {
        return false;
    }
    m_bInitialize = true;
    return true;
}

void WindowsCoreAudio::Terminate()
{

    if (!m_bInitialize)
    {
        return;
    }
    StopRecording();
    StopPlayout();
    lock_guard lg( m_audiolock );
    m_spClientIn.Release();
    m_spClientOut.Release();
    m_recDevIndex = 0;
    m_plyDevIndex = 0;
    m_bInitialize = false;
    m_setEffect.reset();
}

size_t WindowsCoreAudio::GetRecordingDeviceNum() const
{
    return m_CaptureDevices.size();
}

size_t WindowsCoreAudio::GetPlayoutDeviceNum() const
{
    return m_RenderDevices.size();
}

bool WindowsCoreAudio::GetPlayoutDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    lock_guard lg( m_audiolock );

    if ( index >= (int16_t)m_RenderDevices.size() )
    {
        return false;
    }
    if ( index  < 0)
    {
        index = m_plyDevIndex;
    }

    StringCchCopyW( name, kAdmMaxGuidSize - 1, m_RenderDevices[index].szDeviceName );
    StringCchCopyW( guid, kAdmMaxGuidSize - 1, m_RenderDevices[index].szDeviceID );
    return true;
}


bool WindowsCoreAudio::RecordingDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    lock_guard lg( m_audiolock );

    if ( index <= (int16_t)m_CaptureDevices.size() )
    {
        return false;
    }
    if ( index < 0 )
    {
        index = m_recDevIndex;
    }
    StringCchCopyW( name, kAdmMaxGuidSize - 1, m_CaptureDevices[index].szDeviceName );
    StringCchCopyW( guid, kAdmMaxGuidSize - 1, m_CaptureDevices[index].szDeviceID );
    return true;
}


bool WindowsCoreAudio::SetPlayoutDevice( int16_t index )
{
    lock_guard lg( m_audiolock );
    if (m_bInitialize)
    {
        return false;
    }
    if ( index <= (int16_t)m_RenderDevices.size() )
    {
        return false;
    }

    m_plyDevIndex = index;
    return true;
}

bool WindowsCoreAudio::SetRecordingDevice( int16_t index )
{
    lock_guard lg( m_audiolock );
    if (m_bInitialize)
    {
        return false;
    }
    if ( index <= (int16_t)m_CaptureDevices.size() )
    {
        return false;
    }

    m_recDevIndex = index;
    return true;
}

bool WindowsCoreAudio::IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    lock_guard lg( m_audiolock );
    if ( !m_bInitialize )
    {
        return false;
    }

    return IsFormatSupported( m_spClientIn, nSampleRate, nChannels );
}

bool WindowsCoreAudio::IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    lock_guard lg( m_audiolock );
    if ( !m_bInitialize )
    {
        return false;
    }
    return IsFormatSupported( m_spClientOut, nSampleRate, nChannels );
}

bool WindowsCoreAudio::SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    lock_guard lg( m_audiolock );
    if ( !m_bInitialize )
    {
        return false;
    }

    if ( IsFormatSupported(m_spClientIn, nSampleRate, nChannels) )
    {
        m_recSampleRate = nSampleRate;
        m_recChannels = static_cast<uint8_t>( nChannels );
        return true;
    }
    return false;

}

bool WindowsCoreAudio::SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    lock_guard lg( m_audiolock );
    if (!m_bInitialize)
    {
        return false;
    }
    if ( IsFormatSupported( m_spClientOut, nSampleRate, nChannels ) )
    {
        m_plySampleRate = nSampleRate;
        m_plyChannels = static_cast<uint8_t>( nChannels);
        return true;
    }
    return false;

}

bool WindowsCoreAudio::GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    lock_guard lg( m_audiolock );
    if (!m_bInitialize)
    {
        return false;
    }
    if (m_recSampleRate == 0 || m_recChannels == 0)
    {
        return false;
    }
    nSampleRate = m_recSampleRate;
    nChannels = m_recChannels;
    return true;

}

bool WindowsCoreAudio::GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    lock_guard lg( m_audiolock );
    if ( !m_bInitialize )
    {
        return false;
    }

    if ( m_plySampleRate == 0 || m_plyChannels == 0 )
    {
        return false;
    }

    nSampleRate = m_plySampleRate;
    nChannels = m_plyChannels;
    return true;

}

bool WindowsCoreAudio::InitPlayout()
{
    lock_guard lg( m_audiolock );
    if ( !m_bInitialize )
    {
        return false;
    }
    if ( m_playIsInitialized )
    {
        return true;
    }

    if ( m_bUseDMO && m_recIsInitialized )
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
    if ( m_plySampleRate == 44100 )
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
    m_spClientOut.Release();

    DeviceBindTo( eRender, m_plyDevIndex, &m_spClientOut, nullptr, nullptr );
    if ( m_plySampleRate == 0 || m_plyChannels == 0 )
    {
        WAVEFORMATEX* pWfxOut = NULL;
        hr = m_spClientOut->GetMixFormat( &pWfxOut );
        m_plySampleRate = pWfxOut->nSamplesPerSec;
        m_plyChannels = pWfxOut->nChannels;
        ::CoTaskMemFree( pWfxOut );
    }


    WAVEFORMATEX Wfx;
    Wfx.wFormatTag = WAVE_FORMAT_PCM;
    Wfx.wBitsPerSample = 16;
    Wfx.cbSize = 0;
    Wfx.nChannels = m_plyChannels;
    Wfx.nSamplesPerSec = m_plySampleRate;
    Wfx.nBlockAlign = Wfx.nChannels * Wfx.wBitsPerSample / 8;
    Wfx.nAvgBytesPerSec = Wfx.nSamplesPerSec * Wfx.nBlockAlign;

    hr = m_spClientOut->Initialize(
        AUDCLNT_SHAREMODE_SHARED,             // share Audio Engine with other applications
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,    // processing of the audio buffer by the client will be event driven
        hnsBufferDuration,                    // requested buffer capacity as a time value (in 100-nanosecond units)
        0,                                    // periodicity
        &Wfx,                                 // selected wave format
        NULL );                                // session GUID

    IF_FAILED_EXIT( hr );
    

    hr = m_spClientOut->SetEventHandle(
        m_hRenderSamplesReadyEvent );
    IF_FAILED_EXIT( hr );

    // Get an IAudioRenderClient interface.
    m_spRenderClient.Release();// 有可能上次没释放成功
    hr = m_spClientOut->GetService(
        __uuidof( IAudioRenderClient ),
        (void**)&m_spRenderClient );
    IF_FAILED_EXIT( hr );

    m_playIsInitialized = true;
    return true;

Exit:
    m_spRenderClient.Release();
    return false;

}

bool WindowsCoreAudio::InitRecording()
{
    lock_guard lg( m_audiolock );
    if (!m_bInitialize)
    {
        return false;
    }

    if ( m_bUseDMO )
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
    if ( !m_playIsInitialized )
    {
        return false;
    }

    if ( m_hPlayThread != NULL )
    {
        return true;
    }

    if ( m_playing )
    {
        return true;
    }

    {
        std::lock_guard<std::mutex> lg(m_audiolock);
        // Create thread which will drive the rendering.
        m_hPlayThread = CreateThread(
            NULL,
            0,
            WSAPIRenderThread,
            this,
            0,
            NULL );
        if ( m_hPlayThread == NULL )
        {
            return false;
        }

        // Set thread priority to highest possible.
        ::SetThreadPriority( m_hPlayThread, THREAD_PRIORITY_TIME_CRITICAL );
    }  // critScoped

    DWORD ret = WaitForSingleObject( m_hRenderStartedEvent, 1000 );
    if ( ret != WAIT_OBJECT_0 )
    {
        return false;
    }

    m_playing = true;

    return true;
}

bool WindowsCoreAudio::StopPlayout()
{
    if ( !m_playIsInitialized )
    {
        return true;
    }

    {
        std::lock_guard<std::mutex> lg( m_audiolock );

        if ( m_hPlayThread == NULL )
        {
            printf("no rendering stream is active => close down WASAPI only" );
            m_spRenderClient.Release();
            m_playIsInitialized = false;
            m_playing = false;
            return true;
        }

        printf( "closing down the webrtc_core_audio_render_thread..." );
        SetEvent( m_hShutdownRenderEvent );
    }  // critScoped

    DWORD ret = WaitForSingleObject( m_hPlayThread, 2000 );
    if ( ret != WAIT_OBJECT_0 )
    {
        // the thread did not stop as it should
        printf("failed to close down webrtc_core_audio_render_thread" );
        CloseHandle( m_hPlayThread );
        m_hPlayThread = NULL;
        m_playIsInitialized = false;
        m_playing = false;
        return false;
    }

    {
        std::lock_guard<std::mutex> lg( m_audiolock );
        printf("windows_core_audio_render_thread is now closed" );

        // to reset this event manually at each time we finish with it,
        // in case that the render thread has exited before StopPlayout(),
        // this event might be caught by the new render thread within same VoE instance.
        ResetEvent( m_hShutdownRenderEvent );

        m_spRenderClient.Release();
        m_playIsInitialized = false;
        m_playing = false;

        CloseHandle( m_hPlayThread );
        m_hPlayThread = NULL;

        if ( m_bUseDMO && m_recording )
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
    return m_playing;

}

bool WindowsCoreAudio::StartRecording()
{
    if ( !m_recIsInitialized )
    {
        return false;
    }

    if ( m_hRecThread != NULL )
    {
        return true;
    }

    if ( m_recording )
    {
        return true;
    }

    {
        std::lock_guard<std::mutex> lg(m_audiolock);

        // Create thread which will drive the capturing
        LPTHREAD_START_ROUTINE lpStartAddress = WSAPICaptureThread;
        if ( m_bUseDMO )
        {
            // Redirect to the DMO polling method.
            lpStartAddress = WSAPICaptureThreadPollDMO;

            if ( !m_playing )
            {
                // The DMO won't provide us captured output data unless we
                // give it render data to process.
                printf( "Playout must be started before recording when using the built-in AEC" );
                return false;
            }
        }

        assert( m_hRecThread == NULL );
        m_hRecThread = CreateThread( NULL,
                                    0,
                                    lpStartAddress,
                                    this,
                                    0,
                                    NULL );
        if ( m_hRecThread == NULL )
        {
            printf( "failed to create the recording thread" );
            return false;
        }

        // Set thread priority to highest possible
        ::SetThreadPriority( m_hRecThread, THREAD_PRIORITY_TIME_CRITICAL );


    }  // critScoped

    DWORD ret = WaitForSingleObject( m_hCaptureStartedEvent, 1000 );
    if ( ret != WAIT_OBJECT_0 )
    {
        printf( "capturing did not start up properly" );
        return false;
    }
    printf( "capture audio stream has now started..." );


    m_recording = true;
    return true;
}

bool WindowsCoreAudio::StopRecording()
{
    bool err = true;

    if ( !m_recIsInitialized )
    {
        return 0;
    }

    
    {
        lock_guard lg( m_audiolock );
        if ( m_hRecThread == NULL )
        {
            printf( "no capturing stream is active => close down WASAPI only" );
            m_spCaptureClient.Release();

            m_recIsInitialized = false;
            m_recording = false;
            return 0;
        }

        // Stop the driving thread...
        printf( "closing down the webrtc_core_audio_capture_thread..." );
        // Manual-reset event; it will remain signalled to stop all capture threads.
        SetEvent( m_hShutdownCaptureEvent );
    }

    DWORD ret = WaitForSingleObject( m_hRecThread, 2000 );
    if ( ret != WAIT_OBJECT_0 )
    {
        printf( "failed to close down webrtc_core_audio_capture_thread" );
        err = false;
    }
    else
    {
        printf("webrtc_core_audio_capture_thread is now closed" );
    }

    lock_guard lg( m_audiolock );

    ResetEvent( m_hShutdownCaptureEvent ); // Must be manually reset.
    // Ensure that the thread has released these interfaces properly.


    m_recIsInitialized = false;
    m_recording = false;

    // These will create thread leaks in the result of an error,
    // but we can at least resume the call.
    CloseHandle( m_hRecThread );
    m_hRecThread = NULL;

    if ( m_bUseDMO )
    {
        assert( m_spDmo );
        // This is necessary. Otherwise the DMO can generate garbage render
        // audio even after rendering has stopped.
        HRESULT hr = m_spDmo->FreeStreamingResources();
        if ( FAILED( hr ) )
        {
            err = false;
        }
    }

    return err;

}

bool WindowsCoreAudio::Recording() const
{
    return m_recording;
}

void WindowsCoreAudio::SetAudioBufferCallback( AudioBufferProc* pCallback )
{
    m_pBufferProc = pCallback;
}

AudioBufferProc* WindowsCoreAudio::GetAudioBufferCallback()const
{
    return m_pBufferProc;
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
        if ( !m_DMOIsAvailble )
        {
            return false;
        }
        bool var = *(int32_t*)value != 0;
        m_setEffect.set( ID_ENABLE_AEC, var );     
        {
            m_setEffect.set( id, var );
        }
    }
    break;
    }

    if (m_setEffect.to_ulong() & (ID_ENABLE_AEC | ID_ENBALE_AGC | ID_ENBALE_NS | ID_ENBALE_VAD) )
    {
        m_bUseDMO = true;
    }
    else
    {
        m_bUseDMO = false;
    }
    return true;
}

bool WindowsCoreAudio::GetProperty( AudioPropertyID id, void*value )
{
    if ( !m_bUseDMO )
    {
        return false;
    }
    *(int32_t*)value = m_setEffect.test( id ) ? 1 : 0;
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
    assert( m_spDmo != NULL );

    CComPtr<IPropertyStore> ps;
    hr = m_spDmo->QueryInterface( IID_IPropertyStore,
                                  reinterpret_cast<void**>( &ps ) );
    IF_FAILED_EXIT( hr );

    // Set the AEC system mode.
    // SINGLE_CHANNEL_AEC - AEC processing only.
    bool var = m_setEffect.test( ID_ENABLE_AEC );
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
    var = m_setEffect.test( ID_ENBALE_AGC );
    // Disable analog AGC (default enabled).
    hr = SetBoolProperty( ps,
                          MFPKEY_WMAAECMA_MIC_GAIN_BOUNDER,
                          var ? VARIANT_TRUE : VARIANT_FALSE );
    IF_FAILED_EXIT( hr );

    // Disable noise suppression (default enabled).
    // 0 - Disabled, 1 - Enabled
    var = m_setEffect.test( ID_ENBALE_NS );
    hr = SetVtI4Property( ps,
                          MFPKEY_WMAAECMA_FEATR_NS,
                          var ? 1 : 0 );
    IF_FAILED_EXIT( hr );

    var = m_setEffect.test( ID_ENBALE_VAD );
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

    DWORD devIndex = static_cast<uint32_t>( m_plyDevIndex << 16 ) +
        static_cast<uint32_t>( 0x0000ffff & m_recDevIndex );
    hr = SetVtI4Property( ps,
                          MFPKEY_WMAAECMA_DEVICE_INDEXES,
                          devIndex );

Exit:
    return  SUCCEEDED( hr );
}

bool WindowsCoreAudio::InitRecordingMedia()
{
    if ( m_recIsInitialized )
    {
        return true;
    }

    m_spClientIn.Release();
    DeviceBindTo( eCapture, m_recDevIndex, &m_spClientIn, nullptr, nullptr );
    HRESULT hr = S_OK;
    if ( m_recSampleRate == 0 || m_recChannels == 0 )
    {
        WAVEFORMATEX* pWfxIn = NULL;
        hr = m_spClientOut->GetMixFormat( &pWfxIn );
        IF_FAILED_EXIT( hr );
        m_recSampleRate = pWfxIn->nSamplesPerSec;
        m_recChannels = pWfxIn->nChannels;
        ::CoTaskMemFree( pWfxIn );
    }

    // Set wave format
    WAVEFORMATEX Wfx;
    Wfx.wFormatTag = WAVE_FORMAT_PCM;
    Wfx.wBitsPerSample = 16;
    Wfx.cbSize = 0;
    Wfx.nChannels = m_recChannels;
    Wfx.nSamplesPerSec = m_recSampleRate;
    Wfx.nBlockAlign = Wfx.nChannels * Wfx.wBitsPerSample / 8;
    Wfx.nAvgBytesPerSec = Wfx.nSamplesPerSec * Wfx.nBlockAlign;

    // Create a capturing stream.

    hr = m_spClientIn->Initialize(
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
    hr = m_spClientIn->SetEventHandle(
        m_hCaptureSamplesReadyEvent );
    IF_FAILED_EXIT( hr );

    // Get an IAudioCaptureClient interface.
    m_spCaptureClient.Release();// 防止上次没有释放
    hr = m_spClientIn->GetService(
        __uuidof( IAudioCaptureClient ),
        (void**)&m_spCaptureClient );
    IF_FAILED_EXIT( hr );
    m_recIsInitialized = true;
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
    if ( !m_bUseDMO || !m_spDmo )
    {
        return false;
    }
    if ( m_recIsInitialized )
    {
        return true;
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
    if (m_recChannels != ptrWav->nChannels &&
         m_recSampleRate != ptrWav->nSamplesPerSec )
    {
        if ( m_pBufferProc )
        {
            m_pBufferProc->ErrorOccurred( AE_RECORD_FORMAT_CHANGE );
        }
    }
    m_recSampleRate = ptrWav->nSamplesPerSec;
    m_recChannels = ptrWav->nChannels;
    hr = CMediaBuffer::Create( ptrWav->nBlockAlign * ptrWav->nSamplesPerSec/100, (IMediaBuffer**)&m_spMediaBuffer );
    if ( FAILED( hr ) )
    {
        return false;
    }
    // Set the DMO output format parameters.
    hr = m_spDmo->SetOutputType( 0, &mt, 0 );
    ::MoFreeMediaType( &mt );
    if ( FAILED( hr ) )
    {
        return false;
    }

    // Optional, but if called, must be after media types are set.
    hr = m_spDmo->AllocateStreamingResources();
    if ( FAILED( hr ) )
    {
        return false;
    }

    m_recIsInitialized = true;

    return true;
}

DWORD WindowsCoreAudio::DoRenderThread()
{
    bool keepPlaying = true;
    HANDLE waitArray[2] = { m_hShutdownRenderEvent, m_hRenderSamplesReadyEvent };
    HRESULT hr = S_OK;


    // Initialize COM as MTA in this thread.
    ScopedCOMInitializer comInit( ScopedCOMInitializer::kMTA );
    if ( !comInit.succeeded() )
    {
        return 1;
    }

    HANDLE hMmTask = SetThreadPriority( "windows_core_audio_render_thread" );

    m_audiolock.lock();

    // Get size of rendering buffer (length is expressed as the number of audio frames the buffer can hold).
    // This value is fixed during the rendering session.
    //
    UINT32 bufferLength = 0;
    hr = m_spClientOut->GetBufferSize( &bufferLength );
    IF_FAILED_JUMP( hr,Exit );
   
    const size_t playBlockSize = m_plySampleRate / 100;
    const size_t playFrameSize = 2 * m_plyChannels;
    const double endpointBufferSizeMS = 10.0 * ( (double)bufferLength / (double)playBlockSize );

    // Before starting the stream, fill the rendering buffer with silence.
    //
    BYTE *pData = NULL;
    hr = m_spRenderClient->GetBuffer( bufferLength, &pData );
    IF_FAILED_JUMP( hr,Exit );

    hr = m_spRenderClient->ReleaseBuffer( bufferLength, AUDCLNT_BUFFERFLAGS_SILENT );
    IF_FAILED_JUMP( hr, Exit );

    // Start up the rendering audio stream.
    hr = m_spClientOut->Start();
    IF_FAILED_JUMP( hr, Exit );

    m_audiolock.unlock();

    // Set event which will ensure that the calling thread modifies the playing state to true.
    SetEvent( m_hRenderStartedEvent );

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
            m_audiolock.lock();

            // Sanity check to ensure that essential states are not modified
            // during the unlocked period.
            if ( !m_spRenderClient || !m_spClientOut )
            {
                m_audiolock.unlock();
                printf( "output state has been modified during unlocked period\n" );
                goto Exit;
            }

            // Get the number of frames of padding (queued up to play) in the endpoint buffer.
            UINT32 padding = 0;
            hr = m_spClientOut->GetCurrentPadding( &padding );
            IF_FAILED_JUMP( hr,Exit );

            // Derive the amount of available space in the output buffer
            uint32_t framesAvailable = bufferLength - padding;
            // WEBRTC_TRACE(kTraceStream, kTraceAudioDevice, _id, "#avaliable audio frames = %u", framesAvailable);

            // Do we have 10 ms available in the render buffer?
            if ( framesAvailable < playBlockSize )
            {
                // Not enough space in render buffer to store next render packet.
                m_audiolock.unlock();
                break;
            }

            // Write n*10ms buffers to the render buffer
            const uint32_t n10msBuffers = ( framesAvailable / playBlockSize );
            for ( uint32_t n = 0; n < n10msBuffers; n++ )
            {
                // Get pointer (i.e., grab the buffer) to next space in the shared render buffer.
                hr = m_spRenderClient->GetBuffer( playBlockSize, &pData );
                IF_FAILED_JUMP( hr, Exit );

                if ( m_pBufferProc )
                {
                    m_audiolock.unlock();
                    int32_t nSamples = m_pBufferProc->NeedMorePlayoutData( (int16_t*)pData, playBlockSize*playFrameSize );
                    nSamples /= playFrameSize;
                    // Request data to be played out (#bytes = _playBlockSize*_audioFrameSize)
                    
                    m_audiolock.lock();

                    if ( nSamples == -1 )
                    {
                        m_audiolock.unlock();
                        printf( "failed to read data from render client\n" );
                        goto Exit;
                    }

                    // Sanity check to ensure that essential states are not modified during the unlocked period
                    if ( !m_spRenderClient || !m_spClientOut )
                    {
                        m_audiolock.unlock();
                        printf( "output state has been modified during unlocked period\n" );
                        goto Exit;
                    }
                    if ( nSamples != static_cast<int32_t>( playBlockSize ) )
                    {
                        printf("nSamples(%d) != _playBlockSize(%d)\n", nSamples, playBlockSize );
                    }
                }

                DWORD dwFlags( 0 );
                hr = m_spRenderClient->ReleaseBuffer( playBlockSize, dwFlags );
                // See http://msdn.microsoft.com/en-us/library/dd316605(VS.85).aspx
                // for more details regarding AUDCLNT_E_DEVICE_INVALIDATED.
                IF_FAILED_JUMP( hr,Exit );
            }

            m_audiolock.unlock();
        }
    }

    // ------------------ THREAD LOOP ------------------ <<

    Sleep( static_cast<DWORD>( endpointBufferSizeMS + 0.5 ) );
    hr = m_spClientOut->Stop();

Exit:
    if ( FAILED( hr ) )
    {
        m_spClientOut->Stop();
        m_audiolock.unlock();
    }

    RevertThreadPriority( hMmTask );

    m_audiolock.lock();

    if ( keepPlaying )
    {
        if ( m_spClientOut )
        {
            hr = m_spClientOut->Stop();
            m_spClientOut->Reset();
        }
        // Trigger callback from module process thread
        printf( "kPlayoutError message posted: rendering thread has ended pre-maturely" );
    }
    else
    {
        printf( "_Rendering thread is now terminated properly" );
    }

    m_audiolock.unlock();

    return (DWORD)hr;
}

DWORD WindowsCoreAudio::DoCaptureThreadPollDMO()
{
    bool keepRecording = true;

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
    SetEvent( m_hCaptureStartedEvent );
    HRESULT hr = S_OK;
    // >> ---------------------------- THREAD LOOP ----------------------------
    while ( keepRecording )
    {
        // Poll the DMO every 5 ms.
        // (The same interval used in the Wave implementation.)
        DWORD waitResult = WaitForSingleObject( m_hShutdownCaptureEvent, 5 );
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
            std::unique_lock<std::mutex> ul(m_audiolock);
            DWORD dwStatus = 0;
            {
                DMO_OUTPUT_DATA_BUFFER dmoBuffer = { 0 };
                dmoBuffer.pBuffer = m_spMediaBuffer;
                dmoBuffer.pBuffer->AddRef();
                // Poll the DMO for AEC processed capture data. The DMO will
                // copy available data to |dmoBuffer|, and should only return
                // 10 ms frames. The value of |dwStatus| should be ignored.
                hr = m_spDmo->ProcessOutput( 0, 1, &dmoBuffer, &dwStatus );
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
            hr = m_spMediaBuffer->GetBufferAndLength( &data, &bytesProduced );
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
                if ( m_pBufferProc )
                {
                    m_pBufferProc->RecordingDataIsAvailable( (int16_t*)data, kSamplesProduced );
                }
                ul.lock();
            }

            // Reset length to indicate buffer availability.
            hr = m_spMediaBuffer->SetLength( 0 );
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

    return hr;
}



DWORD WindowsCoreAudio::DoCaptureThread()
{
    bool keepRecording = true;
    HANDLE waitArray[2] = { m_hShutdownCaptureEvent, m_hCaptureSamplesReadyEvent };
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

    m_audiolock.lock();

    // Get size of capturing buffer (length is expressed as the number of audio frames the buffer can hold).
    // This value is fixed during the capturing session.
    //
    UINT32 bufferLength = 0;
    if ( !m_spClientIn )
    {
        return 1;
    }
    hr = m_spClientIn->GetBufferSize( &bufferLength );
    IF_FAILED_JUMP( hr,Exit );

    // Allocate memory for sync buffer.
    // It is used for compensation between native 44.1 and internal 44.0 and
    // for cases when the capture buffer is larger than 10ms.
    //
    const UINT32 syncBufferSize = 2 * ( bufferLength * 2 * m_recChannels );
    syncBuffer = new BYTE[syncBufferSize];
    if ( syncBuffer == NULL )
    {
        return (DWORD)E_POINTER;
    }
    printf( "[CAPT] size of sync buffer  : %u [bytes]\n", syncBufferSize );
    const size_t recBlockSize = m_recSampleRate / 100;
    const size_t recAudioFrameSize = 2 * m_recChannels;
    const double endpointBufferSizeMS = 10.0 * ( (double)bufferLength / (double)recAudioFrameSize );
    printf( "[CAPT] endpointBufferSizeMS : %3.2f\n", endpointBufferSizeMS );

    // Start up the capturing stream.
    //
    hr = m_spClientIn->Start();
    IF_FAILED_JUMP( hr,Exit );

    m_audiolock.unlock();

    // Set event which will ensure that the calling thread modifies the recording state to true.
    //
    SetEvent( m_hCaptureStartedEvent );

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

            m_audiolock.lock();

            // Sanity check to ensure that essential states are not modified
            // during the unlocked period.
            if ( !m_spCaptureClient  || !m_spClientIn )
            {
                m_audiolock.unlock();
                printf("input state has been modified during unlocked period\n" );
                goto Exit;
            }

            //  Find out how much capture data is available
            //
            hr = m_spCaptureClient->GetBuffer( &pData,           // packet which is ready to be read by used
                                               &framesAvailable, // #frames in the captured packet (can be zero)
                                               &flags,           // support flags (check)
                                               &recPos,          // device position of first audio frame in data packet
                                               &recTime );        // value of performance counter at the time of recording the first audio frame

            if ( SUCCEEDED( hr ) )
            {
                if ( AUDCLNT_S_BUFFER_EMPTY == hr )
                {
                    // Buffer was empty => start waiting for a new capture notification event
                    m_audiolock.unlock();
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
                hr = m_spCaptureClient->ReleaseBuffer( framesAvailable );
                IF_FAILED_JUMP( hr,Exit );

                syncBufIndex += framesAvailable;

                while ( syncBufIndex >= recBlockSize )
                {
                    m_audiolock.unlock();
                    if (m_pBufferProc)
                    {
                        m_pBufferProc->RecordingDataIsAvailable( (const int16_t*)syncBuffer, recBlockSize*recAudioFrameSize );
                    }
                    m_audiolock.lock();
                    // Sanity check to ensure that essential states are not modified during the unlocked period
                    if ( !m_spCaptureClient || !m_spClientIn )
                    {
                        m_audiolock.unlock();
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

            m_audiolock.unlock();
        }
    }

    // ---------------------------- THREAD LOOP ---------------------------- <<

    if ( m_spClientIn )
    {
        hr = m_spClientIn->Stop();
    }

Exit:
    if ( FAILED( hr ) )
    {
        m_spClientIn->Stop();
        m_audiolock.unlock();
    }

    RevertThreadPriority(hMmTask);

    m_audiolock.lock();

    if ( keepRecording )
    {
        if ( m_spClientIn )
        {
            hr = m_spClientIn->Stop();
            m_spClientIn->Reset();
        }

        // Trigger callback from module process thread
        printf("kRecordingError message posted: capturing thread has ended pre-maturely\n" );
    }
    else
    {
        printf( "_Capturing thread is now terminated properly\n" );
    }

    m_spCaptureClient.Release();

    m_audiolock.unlock();

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




