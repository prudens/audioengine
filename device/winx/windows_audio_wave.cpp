#include "windows_audio_wave.h"
#include <Strsafe.h>
namespace audio_engine
{
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
		if(initialize_)
		{
			return true;
		}
		if(capture_devices_.empty())
		{
			return false;
		}

		if(render_devices_.empty())
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
		if(index == (int16_t)-1)
		{
			index = 0;
		}
		if(render_devices_.empty())
		{
			return false;
		}
		StringCchCopyW( name, kAdmMaxDeviceNameSize - 1, render_devices_[index].szDeviceName );
		StringCchCopyW( guid, kAdmMaxGuidSize - 1, render_devices_[index].szDeviceID );

		return true;
	}

	bool WindowsAudioWave::GetRecordingDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
	{
		if(index == -1)
		{
			index = 0;
		}
		if(capture_devices_.empty())
		{
			return false;
		}
		StringCchCopyW( name, kAdmMaxDeviceNameSize - 1, capture_devices_[index].szDeviceName );
		StringCchCopyW( guid, kAdmMaxGuidSize - 1, capture_devices_[index].szDeviceID );

		return true;
	}

	bool WindowsAudioWave::SetPlayoutDevice( int16_t index )
	{
		if(initialize_)
		{
			return false;
		}
		if(index == -1)
		{
			index = 0;
		}
		if(index >= (int16_t)render_devices_.size())
		{
			return false;
		}
		render_device_index_ = index;
		return true;
	}

	bool WindowsAudioWave::SetRecordingDevice( int16_t index )
	{
		if(initialize_)
		{
			return false;
		}
		if(index == -1)
		{
			index = 0;
		}
		if(index >= (int16_t)capture_devices_.size())
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
		if(res != MMSYSERR_NOERROR)
		{
			TraceWaveInError( res );
			return false;
		}

		return true;
	}

	bool WindowsAudioWave::IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
	{
		if(!initialize_)
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
		if(res != MMSYSERR_NOERROR)
		{
			TraceWaveInError( res );
			return false;
		}

		return true;
	}

	bool WindowsAudioWave::SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )
	{
		if(!IsRecordingFormatSupported( nSampleRate, nChannels ))
		{
			return false;
		}
		capture_sample_rate_ = nSampleRate;
		capture_channel_ = nChannels;
		return true;
	}

	bool WindowsAudioWave::SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )
	{
		if(!IsPlayoutFormatSupported( nSampleRate, nChannels ))
		{
			return false;
		}
		render_sample_rate_ = nSampleRate;
		render_channel_ = nChannels;
		return true;
	}

	bool WindowsAudioWave::GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )
	{
		if(!initialize_)
		{
			return false;
		}
		nSampleRate = capture_sample_rate_;
		nChannels = (uint16_t)capture_channel_;
		return true;
	}

	bool WindowsAudioWave::GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )
	{
		if(!initialize_)
		{
			return false;
		}
		nSampleRate = render_sample_rate_;
		nChannels = render_channel_;
		return true;
	}

	bool WindowsAudioWave::StartPlayout()
	{
		if(!initialize_)
		{
			return false;
		}

		if(playout_thread_handle_ != NULL)
		{
			return true;
		}

		if(playing_)
		{
			return true;
		}

		{
			playing_ = true;
			// Create thread which will drive the rendering.
			playout_thread_handle_ = CreateThread(
				NULL,
				0,
				WSAPIRenderThread,
				this,
				0,
				NULL );
			if(playout_thread_handle_ == NULL)
			{
				playing_ = false;
				return false;
			}

			// Set thread priority to highest possible.
			::SetThreadPriority( playout_thread_handle_, THREAD_PRIORITY_TIME_CRITICAL );
		}  // critScoped

		DWORD ret = WaitForSingleObject( wait_playout_thread_start_handle_, 2000 );
		if(ret != WAIT_OBJECT_0)
		{
			playing_ = false;
			RELEASE_HANDLE( playout_thread_handle_ );
			return false;
		}

		return true;
	}

	bool WindowsAudioWave::StopPlayout()
	{
		if(!initialize_)
		{
			return false;
		}
		playing_ = false; // ֹͣ
		DWORD ret = WaitForSingleObject( playout_thread_handle_, 2000 );
		if(ret != WAIT_OBJECT_0)
		{
			// the thread did not stop as it should
			printf( "failed to close down wave_render_thread" );
			RELEASE_HANDLE( playout_thread_handle_ );
			return false;
		}
		RELEASE_HANDLE( playout_thread_handle_ );

		return true;
	}

	bool WindowsAudioWave::Playing() const
	{
		return playing_;
	}

	bool WindowsAudioWave::StartRecording()
	{
		if(!initialize_)
		{
			return false;
		}

		if(recording_thread_handle_ != NULL)
		{
			return true;
		}

		if(recording_)
		{
			return true;
		}

		{
			recording_ = true;
			// Create thread which will drive the rendering.
			recording_thread_handle_ = CreateThread(
				NULL,
				0,
				WSAPICaptureThread,
				this,
				0,
				NULL );
			if(recording_thread_handle_ == NULL)
			{
				return false;
			}

			// Set thread priority to highest possible.
			::SetThreadPriority( recording_thread_handle_, THREAD_PRIORITY_TIME_CRITICAL );
		}

		DWORD ret = WaitForSingleObject( wait_recording_thread_start_handle_, 2000 );
		if(ret != WAIT_OBJECT_0)
		{
			recording_ = false;
			return false;
		}

		return true;
	}

	bool WindowsAudioWave::StopRecording()
	{
		if(!initialize_)
		{
			return false;
		}
		recording_ = false; // ֹͣ
		DWORD ret = WaitForSingleObject( recording_thread_handle_, 2000 );
		if(ret != WAIT_OBJECT_0)
		{
			// the thread did not stop as it should
			printf( "failed to close down dsound_capture_thread" );
			RELEASE_HANDLE( recording_thread_handle_ );
			return false;
		}
		RELEASE_HANDLE( recording_thread_handle_ );
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
		int devnum = waveInGetNumDevs();
		capture_devices_.reserve( devnum );
		for(int index = 0; index < devnum; index++)
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
			if(res != MMSYSERR_NOERROR)
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
			if(res != MMSYSERR_NOERROR)
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
		int devnum = waveOutGetNumDevs();
		render_devices_.reserve( devnum );
		for(int index = 0; index < devnum; index++)
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
			if(res != MMSYSERR_NOERROR)
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
			if(res != MMSYSERR_NOERROR)
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

	DWORD WINAPI WindowsAudioWave::WSAPIRenderThread( LPVOID context )
	{
		return reinterpret_cast<WindowsAudioWave*>( context )->
			DoRenderThread();
	}

	DWORD WINAPI WindowsAudioWave::WSAPICaptureThread( LPVOID context )
	{
		return reinterpret_cast<WindowsAudioWave*>( context )->
			DoCaptureThread();
	}

	DWORD WindowsAudioWave::DoRenderThread()
	{
		std::vector < char > render_buffer;

		MMRESULT res( MMSYSERR_ERROR );
		HWAVEOUT render_wave_handle = OpenRenderDevice();
		if(!render_wave_handle)
		{
			return 0;
		}

		const size_t bytes_per_frame = 2 * render_channel_ * render_sample_rate_ / 100;// 10ms per frame
		render_buffer.resize( N_BUFFERS_OUT * bytes_per_frame );
		WAVEHDR wave_header_out[N_BUFFERS_OUT];
		for(int n = 0; n < N_BUFFERS_OUT; n++)
		{
			// set up the output wave header
			wave_header_out[n].lpData = reinterpret_cast<LPSTR>( &render_buffer[n* bytes_per_frame] );
			wave_header_out[n].dwBufferLength = bytes_per_frame;
			wave_header_out[n].dwFlags = 0;
			wave_header_out[n].dwLoops = 0;

			// The waveOutPrepareHeader function prepares a waveform-audio data block for playback.
			// The lpData, dwBufferLength, and dwFlags members of the WAVEHDR structure must be set
			// before calling this function.
			//
			res = waveOutPrepareHeader( render_wave_handle, &wave_header_out[n], sizeof( WAVEHDR ) );
			if(MMSYSERR_NOERROR != res)
			{
				TraceWaveOutError( res );
			}
			res = waveOutWrite( render_wave_handle, &wave_header_out[n], sizeof( WAVEHDR ) );
		}

		SetEvent( wait_playout_thread_start_handle_ );
		MSG msg;
		while(playing_ && GetMessage( &msg, 0, 0, 0 ))
		{
			if(MM_WOM_DONE == msg.message)
			{
				WAVEHDR* wheader = (WAVEHDR*)msg.lParam;
				waveOutPrepareHeader( render_wave_handle, wheader, sizeof( WAVEHDR ) );
				wheader->dwBufferLength = bytes_per_frame;
				if(audio_buffer_proc_)
				{
					size_t len = audio_buffer_proc_->NeedMorePlayoutData( wheader->lpData, wheader->dwBufferLength );
					if(len < wheader->dwBufferLength)
					{
						memset( wheader->lpData + len, 0, wheader->dwBufferLength - len );
					}
				}
				else
				{
					printf( "NeedMorePlayoutData :%d\n", wheader->dwBufferLength );
				}
				waveOutPrepareHeader( render_wave_handle, wheader, sizeof( WAVEHDR ) );
				waveOutWrite( render_wave_handle, wheader, sizeof( WAVEHDR ) );
			}
			else if(MM_WOM_CLOSE == msg.message)
			{
				break;
			}
		}

		if(render_wave_handle)
		{
			waveOutClose( render_wave_handle );
		}
		return 0;
	}

	DWORD WindowsAudioWave::DoCaptureThread()
	{
		if(!initialize_)
		{
			return 0;
		}
		if(!recording_)
		{
			return 0;
		}
		// Start by closing any existing wave-input devices
		//
		MMRESULT res( MMSYSERR_ERROR );

		// Store valid handle for the open waveform-audio input device
		HWAVEIN capture_wave_handle = OpenCaptureDevice();
		if(!capture_wave_handle)
		{
			return 0;
		}
		std::vector < char > capture_buffer;
		const size_t bytes_per_frame = 2 * capture_channel_ * capture_sample_rate_ / 100;// 10ms per frame
		capture_buffer.resize( N_BUFFERS_IN * bytes_per_frame );
		WAVEHDR wave_header_in[N_BUFFERS_IN];

		for(int n = 0; n < N_BUFFERS_IN; n++)
		{
			// set up the output wave header
			wave_header_in[n].lpData = reinterpret_cast<LPSTR>( &capture_buffer[n* bytes_per_frame] );
			wave_header_in[n].dwBufferLength = bytes_per_frame;
			wave_header_in[n].dwFlags = 0;
			wave_header_in[n].dwLoops = 0;

			// The waveOutPrepareHeader function prepares a waveform-audio data block for playback.
			// The lpData, dwBufferLength, and dwFlags members of the WAVEHDR structure must be set
			// before calling this function.
			//
			res = waveInPrepareHeader( capture_wave_handle, &wave_header_in[n], sizeof( WAVEHDR ) );
			res = waveInAddBuffer( capture_wave_handle, &wave_header_in[n], sizeof( WAVEHDR ) );
			if(MMSYSERR_NOERROR != res)
			{
				TraceWaveInError( res );
			}

		}

		/////////////////start recording ///////////////////////
		if(MMSYSERR_NOERROR == waveInStart( capture_wave_handle ))
		{
			SetEvent( wait_recording_thread_start_handle_ );
			MSG msg;
			while(GetMessage( &msg, 0, 0, 0 ) && recording_)
			{
				if(MM_WIM_DATA == msg.message)
				{
					WAVEHDR* wheader = (WAVEHDR*)msg.lParam;
					waveInPrepareHeader( capture_wave_handle, wheader, sizeof( WAVEHDR ) );
					if(wheader->dwBytesRecorded == bytes_per_frame)
					{
						if(audio_buffer_proc_)
						{
							audio_buffer_proc_->RecordingDataIsAvailable( wheader->lpData, wheader->dwBytesRecorded );
						}
						else
						{
							printf( "RecordingDataIsAvailable :%d\n", wheader->dwBytesRecorded );
						}
					}
					waveInPrepareHeader( capture_wave_handle, wheader, sizeof( WAVEHDR ) );
					waveInAddBuffer( capture_wave_handle, wheader, sizeof( WAVEHDR ) );
				}
				else if(MM_WIM_CLOSE == msg.message)
				{
					break;
				}
			}
		}

		/////////////////end recording ///////////////////////

		if(capture_wave_handle)
		{
			waveInClose( capture_wave_handle );
		}

		return 0;
	}

	HWAVEOUT WindowsAudioWave::OpenRenderDevice()
	{
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


		if(render_device_index_ >= 0)
		{
			res = waveOutOpen( NULL, render_device_index_, &waveFormat, GetCurrentThreadId(), 0, CALLBACK_THREAD | WAVE_FORMAT_QUERY );
			if(MMSYSERR_NOERROR == res)
			{
				// open the given waveform-audio output device for recording
				res = waveOutOpen( &hWaveOut, render_device_index_, &waveFormat, GetCurrentThreadId(), 0, CALLBACK_THREAD );
			}
		}

		if(!hWaveOut)
		{
			// check if it is possible to open the default communication device (supported on Windows 7)
			res = waveOutOpen( NULL, WAVE_MAPPER, &waveFormat, GetCurrentThreadId(), 0, CALLBACK_THREAD | WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE | WAVE_FORMAT_QUERY );
			if(MMSYSERR_NOERROR == res)
			{
				// if so, open the default communication device for real
				res = waveOutOpen( &hWaveOut, WAVE_MAPPER, &waveFormat, GetCurrentThreadId(), 0, CALLBACK_THREAD | WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE );
			}
			else
			{
				// use default device since default communication device was not avaliable
				res = waveOutOpen( &hWaveOut, WAVE_MAPPER, &waveFormat, GetCurrentThreadId(), 0, CALLBACK_THREAD );
			}
		}

		if(!hWaveOut)
		{
			if(MMSYSERR_NOERROR != res)
			{
				TraceWaveOutError( res );
			}
			return 0;
		}

		// Log information about the acquired output device
		//
		WAVEOUTCAPS caps;

		res = waveOutGetDevCaps( (UINT_PTR)hWaveOut, &caps, sizeof( WAVEOUTCAPS ) );
		if(res != MMSYSERR_NOERROR)
		{
			TraceWaveOutError( res );
		}

		UINT deviceID( 0 );
		res = waveOutGetID( hWaveOut, &deviceID );
		if(res != MMSYSERR_NOERROR)
		{
			TraceWaveOutError( res );
		}

		// Store valid handle for the open waveform-audio output device
		return hWaveOut;
	}

	HWAVEIN WindowsAudioWave::OpenCaptureDevice()
	{
		// Set the input wave format
		//
		MMRESULT res( MMSYSERR_ERROR );

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

		if(capture_device_index_ >= 0)
		{
			// verify settings first
			res = waveInOpen( NULL, capture_device_index_, &waveFormat, GetCurrentThreadId(), 0, CALLBACK_THREAD | WAVE_FORMAT_QUERY );
			if(MMSYSERR_NOERROR == res)
			{
				// open the given waveform-audio input device for recording
				res = waveInOpen( &hWaveIn, capture_device_index_, &waveFormat, GetCurrentThreadId(), 0, CALLBACK_THREAD );
			}
		}
		if(!hWaveIn)
		{
			res = waveInOpen( NULL, WAVE_MAPPER, &waveFormat, GetCurrentThreadId(), 0, CALLBACK_THREAD | WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE | WAVE_FORMAT_QUERY );
			if(MMSYSERR_NOERROR == res)
			{
				// if so, open the default communication device for real
				res = waveInOpen( &hWaveIn, WAVE_MAPPER, &waveFormat, GetCurrentThreadId(), 0, CALLBACK_THREAD | WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE );
			}
			else
			{
				// use default device since default communication device was not avaliable
				res = waveInOpen( &hWaveIn, WAVE_MAPPER, &waveFormat, GetCurrentThreadId(), 0, CALLBACK_THREAD );
			}
		}

		if(MMSYSERR_NOERROR != res)
		{
			TraceWaveInError( res );
			return 0;
		}
		if(!hWaveIn)
		{
			return 0;
		}
		// Log information about the acquired input device
		//
		WAVEINCAPS caps;

		res = waveInGetDevCaps( (UINT_PTR)hWaveIn, &caps, sizeof( WAVEINCAPS ) );
		if(res != MMSYSERR_NOERROR)
		{
			printf( "waveInGetDevCaps() failed (err=%d)", res );
			TraceWaveInError( res );
		}

		UINT deviceID( 0 );
		res = waveInGetID( hWaveIn, &deviceID );
		if(res != MMSYSERR_NOERROR)
		{
			printf( "waveInGetID() failed (err=%d)", res );
			TraceWaveInError( res );
		}

		// Store valid handle for the open waveform-audio input device
		return hWaveIn;
	}
}