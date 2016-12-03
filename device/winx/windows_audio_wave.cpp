#include "windows_audio_wave.h"
#include <Strsafe.h>

#define DRV_RESERVED                      0x0800
#define DRV_QUERYFUNCTIONINSTANCEID       (DRV_RESERVED + 17)
#define DRV_QUERYFUNCTIONINSTANCEIDSIZE   (DRV_RESERVED + 18)

WindowsAudioWave::WindowsAudioWave()
{
    GetCaptureDeviceList();
    GetRenderDeviceList();
    wait_playout_thread_start_handle_ = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    wait_recording_thread_start_handle_ = ::CreateEvent( NULL, FALSE, FALSE, NULL );
}

WindowsAudioWave::~WindowsAudioWave()
{
    RELEASE_HANDLE( wait_playout_thread_start_handle_ );
    RELEASE_HANDLE( wait_recording_thread_start_handle_ );

}

void WindowsAudioWave::Release()
{
    delete this;
}

bool WindowsAudioWave::Initialize()
{
    if (initialize_)
    {
        return true;
    }
    if (!capture_devices_.empty())
    {
        return false;
    }

    if (!render_devices_.empty())
    {
        return false;
    }
    initialize_ = true;
    return true;
}

void WindowsAudioWave::Terminate()
{
    StopPlayout();
    StopRecording();
}

size_t WindowsAudioWave::GetRecordingDeviceNum() const
{
    return capture_devices_.size();
}

size_t WindowsAudioWave::GetPlayoutDeviceNum() const
{
    return render_devices_.size();
}

bool WindowsAudioWave::GetPlayoutDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    if (index == (int16_t)-1)
    {
        index = 0;
    }
    if (render_devices_.empty())
    {
        return false;
    }
    StringCchCopyW( name, kAdmMaxDeviceNameSize - 1, render_devices_[index].szDeviceName );
    StringCchCopyW( guid, kAdmMaxGuidSize - 1, render_devices_[index].szDeviceID );

    return true;
}

bool WindowsAudioWave::GetRecordingDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
{
    if ( index == -1 )
    {
        index = 0;
    }
    if ( capture_devices_.empty() )
    {
        return false;
    }
    StringCchCopyW( name, kAdmMaxDeviceNameSize - 1, capture_devices_[index].szDeviceName );
    StringCchCopyW( guid, kAdmMaxGuidSize - 1, capture_devices_[index].szDeviceID );

    return true;
}

bool WindowsAudioWave::SetPlayoutDevice( int16_t index )
{
    if ( initialize_ )
    {
        return false;
    }
    if (index == -1)
    {
        index = 0;
    }
    if (index >= (int16_t)render_devices_.size())
    {
        return false;
    }
    render_device_index_ = index;
    return true;
}

bool WindowsAudioWave::SetRecordingDevice( int16_t index )
{
    if ( initialize_ )
    {
        return false;
    }
    if ( index == -1 )
    {
        index = 0;
    }
    if ( index >= (int16_t)capture_devices_.size() )
    {
        return false;
    }
    capture_device_index_ = index;
    return true;
}

bool WindowsAudioWave::IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = nChannels;  // mono <=> 1, stereo <=> 2
    waveFormat.nSamplesPerSec = nSampleRate;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = waveFormat.nChannels * ( waveFormat.wBitsPerSample / 8 );
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    auto res = waveInOpen( NULL, capture_device_index_, &waveFormat, 0, 0, CALLBACK_NULL | WAVE_FORMAT_QUERY );
    if ( res != MMSYSERR_NOERROR )
    {
        TraceWaveInError( res );
        return false;
    }

    return true;
}

bool WindowsAudioWave::IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
{
    if (!initialize_)
    {
        return false;
    }
    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = nChannels;  // mono <=> 1, stereo <=> 2
    waveFormat.nSamplesPerSec = nSampleRate;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = waveFormat.nChannels * ( waveFormat.wBitsPerSample / 8 );
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    auto res = waveOutOpen( NULL, capture_device_index_, &waveFormat, 0, 0, CALLBACK_NULL | WAVE_FORMAT_QUERY );
    if ( res != MMSYSERR_NOERROR )
    {
        TraceWaveInError( res );
        return false;
    }

    return true;
}

bool WindowsAudioWave::SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    if ( !IsRecordingFormatSupported( nSampleRate, nChannels ) )
    {
        return false;
    }
    capture_sample_rate_ = nSampleRate;
    capture_channel_ = nChannels;
    return true;
}

bool WindowsAudioWave::SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    if (!IsPlayoutFormatSupported(nSampleRate,nChannels))
    {
        return false;
    }
    render_sample_rate_ = nSampleRate;
    render_channel_ = nChannels;
    return true;
}

bool WindowsAudioWave::GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    if (!initialize_)
    {
        return false;
    }
    nSampleRate = capture_sample_rate_;
    nChannels = (uint16_t)capture_channel_;
    return true;
}

bool WindowsAudioWave::GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    if (!initialize_)
    {
        return false;
    }
    nSampleRate = render_sample_rate_;
    nChannels = render_channel_;
    return true;
}

bool WindowsAudioWave::InitPlayout()
{
    if (render_wave_handle_)
    {
        waveOutClose( render_wave_handle_ );
        render_wave_handle_ = nullptr;
    }
    HWAVEOUT hWaveOut( NULL );
    MMRESULT res( MMSYSERR_ERROR );

    // Set the output wave format
    //
    WAVEFORMATEX waveFormat;

    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = render_channel_;  // mono <=> 1, stereo <=> 2
    waveFormat.nSamplesPerSec = render_sample_rate_;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = waveFormat.nChannels * ( waveFormat.wBitsPerSample / 8 );
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;


    if (render_device_index_ >= 0 )
    {

        res = waveOutOpen( NULL, render_device_index_, &waveFormat, 0, 0, CALLBACK_NULL | WAVE_FORMAT_QUERY );
        if ( MMSYSERR_NOERROR == res )
        {
            // open the given waveform-audio output device for recording
            res = waveOutOpen( &hWaveOut, render_device_index_, &waveFormat, 0, 0, CALLBACK_NULL );
        }
    }

    if (!hWaveOut)
    {
        // check if it is possible to open the default communication device (supported on Windows 7)
        res = waveOutOpen( NULL, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL | WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE | WAVE_FORMAT_QUERY );
        if ( MMSYSERR_NOERROR == res )
        {
            // if so, open the default communication device for real
            res = waveOutOpen( &hWaveOut, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL | WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE );
        }
        else
        {
            // use default device since default communication device was not avaliable
            res = waveOutOpen( &hWaveOut, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL );
        }
    }

    if (!hWaveOut)
    {
        if ( MMSYSERR_NOERROR != res )
        {
            TraceWaveOutError( res );
        }
        return false;
    }

    // Log information about the acquired output device
    //
    WAVEOUTCAPS caps;

    res = waveOutGetDevCaps( (UINT_PTR)hWaveOut, &caps, sizeof( WAVEOUTCAPS ) );
    if ( res != MMSYSERR_NOERROR )
    {
        TraceWaveOutError( res );
    }

    UINT deviceID( 0 );
    res = waveOutGetID( hWaveOut, &deviceID );
    if ( res != MMSYSERR_NOERROR )
    {
        TraceWaveOutError( res );
    }

    // Store valid handle for the open waveform-audio output device
    render_wave_handle_ = hWaveOut;

    const size_t bytes_per_frame = 2 * render_channel_ * render_sample_rate_ / 100;// 10ms per frame
    ply_buffer_.resize( N_BUFFERS_OUT * bytes_per_frame ); 

    for ( int n = 0; n < N_BUFFERS_OUT; n++ )
    {
        // set up the output wave header
        wave_header_out_[n].lpData = reinterpret_cast<LPSTR>( ply_buffer_.data() + n* bytes_per_frame );
        wave_header_out_[n].dwBufferLength = bytes_per_frame;
        wave_header_out_[n].dwFlags = 0;
        wave_header_out_[n].dwLoops = 0;

        // The waveOutPrepareHeader function prepares a waveform-audio data block for playback.
        // The lpData, dwBufferLength, and dwFlags members of the WAVEHDR structure must be set
        // before calling this function.
        //
        res = waveOutPrepareHeader( render_wave_handle_, &wave_header_out_[n], sizeof( WAVEHDR ) );
        if ( MMSYSERR_NOERROR != res )
        {
            TraceWaveOutError( res );
        }

        // perform extra check to ensure that the header is prepared
        if ( wave_header_out_[n].dwFlags != WHDR_PREPARED )
        {
            printf( "waveOutPrepareHeader(%d) failed (dwFlags != WHDR_PREPARED)", n );
        }
    }

    return true;
}

bool WindowsAudioWave::InitRecording()
{
    if (!initialize_)
    {
        return false;
    }
    if (init_recording_)
    {
        return true;
    }

    // Start by closing any existing wave-input devices
    //
    MMRESULT res( MMSYSERR_ERROR );

    if ( capture_wave_handle_ )
    {
        res = waveInClose( capture_wave_handle_ );
        capture_wave_handle_ = nullptr;
        if ( MMSYSERR_NOERROR != res )
        {
            TraceWaveInError( res );
        }
    }

    // Set the input wave format
    //
    WAVEFORMATEX waveFormat;

    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = capture_channel_;  // mono <=> 1, stereo <=> 2
    waveFormat.nSamplesPerSec = capture_sample_rate_;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = waveFormat.nChannels * ( waveFormat.wBitsPerSample / 8 );
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    // Open the given waveform-audio input device for recording
    //
    HWAVEIN hWaveIn( NULL );

    if ( capture_device_index_ >= 0 )
    {
        // verify settings first
        res = waveInOpen( NULL, capture_device_index_, &waveFormat, 0, 0, CALLBACK_NULL | WAVE_FORMAT_QUERY );
        if ( MMSYSERR_NOERROR == res )
        {
            // open the given waveform-audio input device for recording
            res = waveInOpen( &hWaveIn, capture_device_index_, &waveFormat, 0, 0, CALLBACK_NULL );
        }
    }
    if (!hWaveIn)
    {
        res = waveInOpen( NULL, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL | WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE | WAVE_FORMAT_QUERY );
        if ( MMSYSERR_NOERROR == res )
        {
            // if so, open the default communication device for real
            res = waveInOpen( &hWaveIn, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL | WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE );
        }
        else
        {
            // use default device since default communication device was not avaliable
            res = waveInOpen( &hWaveIn, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL );
        }
    }
   
    if ( MMSYSERR_NOERROR != res )
    {
        TraceWaveInError( res );
        return false;
    }
    if (!hWaveIn)
    {
        return false;
    }
    // Log information about the acquired input device
    //
    WAVEINCAPS caps;

    res = waveInGetDevCaps( (UINT_PTR)hWaveIn, &caps, sizeof( WAVEINCAPS ) );
    if ( res != MMSYSERR_NOERROR )
    {
        printf( "waveInGetDevCaps() failed (err=%d)", res );
        TraceWaveInError( res );
    }

    UINT deviceID( 0 );
    res = waveInGetID( hWaveIn, &deviceID );
    if ( res != MMSYSERR_NOERROR )
    {
        printf( "waveInGetID() failed (err=%d)", res );
        TraceWaveInError( res );
    }

    // Store valid handle for the open waveform-audio input device
    capture_wave_handle_ = hWaveIn;
    init_recording_ = true;
    return true;
}

bool WindowsAudioWave::StartPlayout()
{
    return true;
}

bool WindowsAudioWave::StopPlayout()
{
    playing_ = false;
    return true;
}

bool WindowsAudioWave::Playing() const
{
    return playing_;
}

bool WindowsAudioWave::StartRecording()
{
    return false;
}

bool WindowsAudioWave::StopRecording()
{
    return true;
}

bool WindowsAudioWave::Recording() const
{
    return recording_;
}

void WindowsAudioWave::SetAudioBufferCallback( AudioBufferProc* pCallback )
{
    audio_buffer_proc_ = pCallback;
}

AudioBufferProc* WindowsAudioWave::GetAudioBufferCallback() const
{
    return audio_buffer_proc_;
}

bool WindowsAudioWave::SetPropertie( AudioPropertyID /*id*/, void* )
{
    return false;
}

bool WindowsAudioWave::GetProperty( AudioPropertyID /*id*/, void* )
{
    return false;
}

void WindowsAudioWave::GetCaptureDeviceList()
{
    capture_devices_.clear();
    int devnum = GetRecordingDeviceNum();
    capture_devices_.reserve( devnum );
    for ( int index = 0; index < devnum; index++ )
    {
        AUDIO_DEVICE_INFO dev;
        WAVEINCAPSW caps;
        waveInGetDevCapsW( index, &caps, sizeof( WAVEINCAPSW ) );
        StringCchCopyW( dev.szDeviceName, kAdmMaxDeviceNameSize - 1, caps.szPname );
        size_t cbEndpointId( 0 );

        // Get the size (including the terminating null) of the endpoint ID string of the waveOut device.
        // Windows Vista supports the DRV_QUERYFUNCTIONINSTANCEIDSIZE and DRV_QUERYFUNCTIONINSTANCEID messages.
        auto res = waveInMessage( (HWAVEIN)IntToPtr( index ),
                                   DRV_QUERYFUNCTIONINSTANCEIDSIZE,
                                   (DWORD_PTR)&cbEndpointId, NULL );
        if ( res != MMSYSERR_NOERROR )
        {
            StringCchCopyW( dev.szDeviceID, kAdmMaxGuidSize - 1, caps.szPname );
            capture_devices_.push_back( dev );
            continue;
        }

        // waveOutMessage(DRV_QUERYFUNCTIONINSTANCEIDSIZE) worked => we are on a Vista or Windows 7 device

        WCHAR *pstrEndpointId = NULL;
        pstrEndpointId = (WCHAR*)CoTaskMemAlloc( cbEndpointId );

        // Get the endpoint ID string for this waveOut device.
        res = waveInMessage( (HWAVEIN)IntToPtr( index ),
                              DRV_QUERYFUNCTIONINSTANCEID,
                              (DWORD_PTR)pstrEndpointId,
                              cbEndpointId );
        if ( res != MMSYSERR_NOERROR )
        {
            StringCchCopyW( dev.szDeviceID, kAdmMaxGuidSize - 1, caps.szPname );
            CoTaskMemFree( pstrEndpointId );
            capture_devices_.push_back( dev );
            continue;
        }
        StringCchCopyW( dev.szDeviceID, kAdmMaxGuidSize - 1, pstrEndpointId );
        CoTaskMemFree( pstrEndpointId );
        capture_devices_.push_back( dev );
    }

}

void WindowsAudioWave::GetRenderDeviceList()
{
    render_devices_.clear();
    int devnum = GetPlayoutDeviceNum();
    render_devices_.reserve( devnum );
    for ( int index = 0; index < devnum; index++ )
    {
        AUDIO_DEVICE_INFO dev;
        WAVEOUTCAPSW caps;
        waveOutGetDevCapsW( index, &caps, sizeof( WAVEOUTCAPSW ) );
        StringCchCopyW( dev.szDeviceName, kAdmMaxDeviceNameSize - 1, caps.szPname );
        size_t cbEndpointId( 0 );

        // Get the size (including the terminating null) of the endpoint ID string of the waveOut device.
        // Windows Vista supports the DRV_QUERYFUNCTIONINSTANCEIDSIZE and DRV_QUERYFUNCTIONINSTANCEID messages.
        auto res = waveOutMessage( (HWAVEOUT)IntToPtr( index ),
                                   DRV_QUERYFUNCTIONINSTANCEIDSIZE,
                                   (DWORD_PTR)&cbEndpointId, NULL );
        if ( res != MMSYSERR_NOERROR )
        {
            StringCchCopyW( dev.szDeviceID, kAdmMaxGuidSize - 1, caps.szPname );
            render_devices_.push_back( dev );
            continue;
        }

        // waveOutMessage(DRV_QUERYFUNCTIONINSTANCEIDSIZE) worked => we are on a Vista or Windows 7 device

        WCHAR *pstrEndpointId = NULL;
        pstrEndpointId = (WCHAR*)CoTaskMemAlloc( cbEndpointId );

        // Get the endpoint ID string for this waveOut device.
        res = waveOutMessage( (HWAVEOUT)IntToPtr( index ),
                              DRV_QUERYFUNCTIONINSTANCEID,
                              (DWORD_PTR)pstrEndpointId,
                              cbEndpointId );
        if ( res != MMSYSERR_NOERROR )
        {
            StringCchCopyW( dev.szDeviceID, kAdmMaxGuidSize - 1, caps.szPname );
            CoTaskMemFree( pstrEndpointId );
            render_devices_.push_back( dev );
            continue;
        }
        StringCchCopyW( dev.szDeviceID, kAdmMaxGuidSize - 1, pstrEndpointId );
        CoTaskMemFree( pstrEndpointId );
        render_devices_.push_back( dev );
    }

}


void WindowsAudioWave::TraceWaveInError( MMRESULT error ) const
{
    TCHAR buf[MAXERRORLENGTH];
    TCHAR msg[MAXERRORLENGTH];

    StringCchPrintf( buf, MAXERRORLENGTH, TEXT( "Error details: " ) );
    waveInGetErrorText( error, msg, MAXERRORLENGTH );
    StringCchCat( buf, MAXERRORLENGTH, msg );
    printf( "%s", buf );
}


void WindowsAudioWave::TraceWaveOutError( MMRESULT error ) const
{
    TCHAR buf[MAXERRORLENGTH];
    TCHAR msg[MAXERRORLENGTH];

    StringCchPrintf( buf, MAXERRORLENGTH, TEXT( "Error details: " ) );
    waveOutGetErrorText( error, msg, MAXERRORLENGTH );
    StringCchCat( buf, MAXERRORLENGTH, msg );
    printf( "%s", buf );
}