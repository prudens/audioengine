#include "windows_audio_dsound.h"
#include <dsound.h>
#include <Strsafe.h>
#include <algorithm>
#include "base\time_cvt.hpp"
#include <thread>
namespace audio_engine
{
	WindowsAudioDSound::WindowsAudioDSound()
	{
		HRESULT hr = S_OK;
		hr = DirectSoundEnumerate( &WindowsAudioDSound::EnumCaptrueCallback, &render_devices_ );
		hr = DirectSoundCaptureEnumerate( &WindowsAudioDSound::EnumCaptrueCallback, &capture_devices_ );
		wait_playout_thread_start_handle_ = ::CreateEvent( NULL, FALSE, FALSE, NULL );
		wait_recording_thread_start_handle_ = ::CreateEvent( NULL, FALSE, FALSE, NULL );
	}

	WindowsAudioDSound::~WindowsAudioDSound()
	{
		Terminate();
		RELEASE_HANDLE( wait_recording_thread_start_handle_ );
		RELEASE_HANDLE( wait_recording_thread_start_handle_ );
	}

	void WindowsAudioDSound::Release()
	{
		delete this;
	}

	bool WindowsAudioDSound::Initialize()
	{
		if(initialize_)
		{
			return true;
		}

		HRESULT hr = S_OK;
		GUID guid = GUID_NULL;

		{// render device initialize
			if(render_device_index_ >= 0 && render_device_index_ < (int)render_devices_.size())
			{
				CLSIDFromString( render_devices_[render_device_index_].szDeviceID, &guid );
			}
			hr = DirectSoundCreate( &guid, &render_direct_sound_, NULL );
			if(FAILED( hr ))
			{
				return false;
			}
			HWND hWnd = GetDesktopWindow();
			hr = render_direct_sound_->SetCooperativeLevel( hWnd, DSSCL_PRIORITY );

		}

		hr = S_OK;
		guid = GUID_NULL;

		{//capture device initialize
			if(capture_device_index_ >= 0 && capture_device_index_ < (int)capture_devices_.size())
			{
				CLSIDFromString( capture_devices_[capture_device_index_].szDeviceID, &guid );
			}
			hr = DirectSoundCaptureCreate( &guid, &capture_direct_sound_, NULL );
			if(FAILED( hr ))
			{
				return false;
			}
		}

		initialize_ = true;
		return initialize_;
	}

	void WindowsAudioDSound::Terminate()
	{
		if(!initialize_)
		{
			return;
		}
		StopRecording();
		StopPlayout();

		if(capture_direct_sound_buf_)
		{
			capture_direct_sound_buf_->Release();
		}
		if(render_direct_sound_buf_)
		{
			render_direct_sound_buf_->Release();
		}

		if(capture_direct_sound_)
		{
			capture_direct_sound_->Release();
		}
		if(render_direct_sound_)
		{
			render_direct_sound_->Release();
		}

		playing_ = false;
		recording_ = false;
		initialize_ = false;
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
		if(index >= (int16_t)render_devices_.size())
		{
			return false;
		}
		if(index < 0)
		{
			for(size_t i = 0; i < render_devices_.size(); i++)
			{
				if(render_devices_[i].bDefaultDevice)
				{
					index = static_cast<int16_t>( i );
					break;
				}
			}
		}

		StringCchCopyW( name, kAdmMaxGuidSize - 1, render_devices_[index].szDeviceName );
		StringCchCopyW( guid, kAdmMaxGuidSize - 1, render_devices_[index].szDeviceID );
		return true;
	}

	bool WindowsAudioDSound::GetRecordingDeviceName( int16_t index, wchar_t name[kAdmMaxDeviceNameSize], wchar_t guid[kAdmMaxGuidSize] )
	{
		if(index >= (int16_t)capture_devices_.size())
		{
			return false;
		}
		if(index < 0)
		{
			for(size_t i = 0; i < capture_devices_.size(); i++)
			{
				if(capture_devices_[i].bDefaultDevice)
				{
					index = static_cast<int16_t>( i );
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
		if(initialize_)
		{
			return false;
		}
		if(index >= (int16_t)render_devices_.size() || index < 0)
		{
			return false;
		}

		render_device_index_ = index;
		return true;
	}

	bool WindowsAudioDSound::SetRecordingDevice( int16_t index )
	{
		if(initialize_)
		{
			return false;
		}
		if(index >= (int16_t)capture_devices_.size())
		{
			return false;
		}

		capture_device_index_ = index;
		return true;
	}

	bool WindowsAudioDSound::IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
	{
		if(!initialize_)
		{
			return false;
		}
		return TrySetRecordingFormat( nSampleRate, nChannels, false );
	}

	bool WindowsAudioDSound::IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels )
	{
		if(!initialize_)
		{
			return false;
		}
		return TrySetPlayoutFormat( nSampleRate, nChannels, false );
	}

	bool WindowsAudioDSound::SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )
	{
		if(!initialize_)
		{
			return false;
		}
		if(!TrySetRecordingFormat( nSampleRate, nChannels, false ))
		{
			return false;
		}
		capture_sample_rate_ = nSampleRate;
		capture_channel_ = nChannels;
		return true;
	}

	bool WindowsAudioDSound::SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )
	{
		if(!initialize_)
		{
			return false;
		}
		bool ret = TrySetPlayoutFormat( nSampleRate, nChannels, false );
		if(!ret)
		{
			return false;
		}
		render_channel_ = nChannels;
		render_sample_rate_ = nSampleRate;
		return true;
	}

	bool WindowsAudioDSound::GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )
	{
		if(!initialize_)
		{
			return false;
		}
		nSampleRate = static_cast<uint32_t>( capture_sample_rate_ );
		nChannels = static_cast<uint16_t>( capture_channel_ );
		return true;
	}

	bool WindowsAudioDSound::GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )
	{
		if(!initialize_)
		{
			return false;
		}
		nSampleRate = static_cast<uint32_t>( render_sample_rate_ );
		nChannels = static_cast<uint16_t>( render_channel_ );
		return true;
	}


	bool WindowsAudioDSound::StartPlayout()
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

		if(!TrySetPlayoutFormat( (uint32_t)render_sample_rate_, (uint16_t)render_channel_, true ))
		{
			return false;
		}

		{
			playing_ = true;
			std::lock_guard<std::mutex> lg( render_lock_ );
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
				return false;
			}

			// Set thread priority to highest possible.
			::SetThreadPriority( playout_thread_handle_, THREAD_PRIORITY_TIME_CRITICAL );
		}  // critScoped

		DWORD ret = WaitForSingleObject( wait_playout_thread_start_handle_, 1000 );
		if(ret != WAIT_OBJECT_0)
		{
			playing_ = false;
			RELEASE_HANDLE( playout_thread_handle_ );
			return false;
		}

		return true;
	}

	bool WindowsAudioDSound::StopPlayout()
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
			printf( "failed to close down dsound_capture_thread" );
			RELEASE_HANDLE( playout_thread_handle_ );
			return false;
		}
		RELEASE_HANDLE( playout_thread_handle_ );

		return true;
	}

	bool WindowsAudioDSound::Playing() const
	{
		return playing_;
	}

	bool WindowsAudioDSound::StartRecording()
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

		if(!TrySetRecordingFormat( (uint32_t)capture_sample_rate_, (uint16_t)capture_channel_, true ))
		{
			return false;
		}

		{
			std::lock_guard<std::mutex> lg( render_lock_ );
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

		DWORD ret = WaitForSingleObject( wait_recording_thread_start_handle_, 1000 );
		if(ret != WAIT_OBJECT_0)
		{
			recording_ = false;
			return false;
		}

		return true;
	}

	bool WindowsAudioDSound::StopRecording()
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

	bool WindowsAudioDSound::Recording() const
	{
		return recording_;
	}

	void WindowsAudioDSound::SetAudioBufferCallback( AudioBufferProc* pCallback )
	{
		audio_buffer_proc_ = pCallback;
	}

	AudioBufferProc* WindowsAudioDSound::GetAudioBufferCallback() const
	{
		return audio_buffer_proc_;
	}

	bool WindowsAudioDSound::SetPropertie( AudioPropertyID /*id*/, void* )
	{

		return false;
	}

	bool WindowsAudioDSound::GetProperty( AudioPropertyID /*id*/, void* )
	{
		return false;
	}


	BOOL CALLBACK WindowsAudioDSound::EnumCaptrueCallback( GUID* pGUID, LPCSTR szDesc, LPCSTR /*szDrvName*/, void* pContext )
	{
		AUDIO_DEVICE_INFO_LIST *devices = (AUDIO_DEVICE_INFO_LIST*)pContext;

		if(pGUID)
		{
			AUDIO_DEVICE_INFO info;
			StringFromGUID2( *pGUID, info.szDeviceID, sizeof( info.szDeviceID ) / sizeof( info.szDeviceID[0] ) );
			std::mbstowcs( info.szDeviceName, szDesc, sizeof( info.szDeviceName ) / sizeof( info.szDeviceName[0] ) );
			devices->push_back( info );
		}

		return TRUE;
	}

	DWORD  WindowsAudioDSound::DoRenderThread()
	{
		if(!initialize_)
		{
			return 0;
		}

		WAVEFORMATEX wfx;
		wfx.cbSize = 0;
		HRESULT hr = S_OK;
		hr = render_direct_sound_buf_->GetFormat( &wfx, sizeof( wfx ), nullptr );
		int           frameSize = render_sample_rate_ / 100 * wfx.nChannels * 2;
		int           framedelay = ( std::max )( 5UL, 1000 * frameSize / wfx.nAvgBytesPerSec );
		unsigned long dwBufferSize = ( std::max )( 2, ( MAX_DELAY / framedelay ) ) * frameSize + frameSize;
		unsigned long safe_delay = std::max<unsigned long>( 2, ( SAFE_DELAY / framedelay ) ) * frameSize;
		unsigned long min_delay = std::max<unsigned long>( 2, ( MIN_DELAY / framedelay ) ) * frameSize;
		//fill silience
		{
			void*         plockedbuffer = NULL;
			unsigned long dwlockedsize = dwBufferSize;
			if(FAILED( render_direct_sound_buf_->Lock( 0, 0, &plockedbuffer, &dwlockedsize, NULL, NULL, DSBLOCK_ENTIREBUFFER ) ))
				goto err;

			//   memset( (BYTE*)plockedbuffer, 0, dwlockedsize );
			render_direct_sound_buf_->Unlock( plockedbuffer, dwlockedsize, NULL, 0 );
		}

		render_direct_sound_buf_->SetCurrentPosition( 0 );
		if(FAILED( render_direct_sound_buf_->Play( 0, 0, DSBPLAY_LOOPING ) ))
			goto err;

		// play data
		{
			unsigned long nNextOffset = frameSize;
			unsigned long dwCurDevPos = 0;
			unsigned long dwCurDataPos = 0;

			unsigned long filldelay_ttl = MIN_DELAY;
			DWORD          less_timepoint = ::GetTickCount();

			SetEvent( wait_playout_thread_start_handle_ );
			while(playing_)
			{
				if(FAILED( hr = render_direct_sound_buf_->GetCurrentPosition( &dwCurDevPos, &dwCurDataPos ) ))
				{
					break;
				}

				LONG datasize = nNextOffset - dwCurDataPos;
				if(datasize < 0)
					datasize += dwBufferSize;

				if(datasize <= (long)min_delay)
				{
					if(datasize < (long)safe_delay)
						filldelay_ttl = std::min<unsigned long>( dwBufferSize - frameSize, filldelay_ttl + frameSize );
					less_timepoint = ::GetTickCount();
				}
				else if(GetTickCount() - less_timepoint > 4000)
				{
					filldelay_ttl = std::max<unsigned long>( min_delay, filldelay_ttl - frameSize );
					less_timepoint = ::GetTickCount();
				}


				if(datasize <= (LONG)filldelay_ttl)
				{
					void*         pbCaptureData = NULL;
					unsigned long dwLockedSize = 0;
					hr = render_direct_sound_buf_->Lock( nNextOffset, frameSize, &pbCaptureData, &dwLockedSize, NULL, NULL, 0L );

					if(FAILED( hr ))
						break;

					if(DSBSTATUS_BUFFERLOST == hr)
					{
						while(playing_)
						{
							hr = render_direct_sound_buf_->Restore();
							if(hr == DSERR_BUFFERLOST)
								Sleep( 5 );
							else
								break;
						}


						hr = render_direct_sound_buf_->Lock( nNextOffset, frameSize, &pbCaptureData, &dwLockedSize, NULL, NULL, 0L );

						if(FAILED( hr ))
							break;
					}

					LONG delayLength = nNextOffset - dwCurDevPos;
					if(delayLength < 0)
						delayLength += dwBufferSize;

					size_t nReaded = 0;
					if(audio_buffer_proc_)
					{
						nReaded = audio_buffer_proc_->NeedMorePlayoutData( pbCaptureData, dwLockedSize );

					}
					// printf( "NeedMorePlayoutData len=%d\n", dwLockedSize );
					if(0 == nReaded)
						memset( pbCaptureData, 0, frameSize );

					render_direct_sound_buf_->Unlock( pbCaptureData, dwLockedSize, NULL, NULL );

					nNextOffset += frameSize;
					if(nNextOffset == dwBufferSize)
						nNextOffset = 0;
				}
				else
				{
					Sleep( framedelay / 2 );
				}
			}

		}
		render_direct_sound_buf_->Stop();
	err:
		if(render_direct_sound_buf_)
		{
			render_direct_sound_buf_->Release();
			render_direct_sound_buf_ = nullptr;
		}
		return 0;
	}


	DWORD WINAPI WindowsAudioDSound::WSAPICaptureThread( LPVOID context )
	{
		return reinterpret_cast<WindowsAudioDSound*>( context )->
			DoCaptureThread();
	}

	bool WindowsAudioDSound::TrySetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels, bool alloc_buffer )
	{
		if(!initialize_)
		{
			return false;
		}
		// Get the primary buffer 
		HRESULT hr = S_OK;

		WAVEFORMATEX wfx;
		wfx.cbSize = 0;
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = nChannels;
		wfx.nSamplesPerSec = nSampleRate;
		wfx.wBitsPerSample = 16;
		wfx.nBlockAlign = wfx.wBitsPerSample * nChannels / 8;
		wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

		int frameSize = wfx.nSamplesPerSec / 100 * wfx.nChannels * 2;
		int framedelay = std::max<int>( 5, 1000 * frameSize / wfx.nAvgBytesPerSec );
		DWORD dwBufferSize = std::max<DWORD>( 2, ( MAX_DELAY / framedelay ) ) * frameSize + frameSize;

		DSCBUFFERDESC                 dscbd;
		memset( &dscbd, 0, sizeof( DSCBUFFERDESC ) );
		dscbd.dwSize = sizeof( DSCBUFFERDESC );
		dscbd.dwBufferBytes = dwBufferSize;
		dscbd.lpwfxFormat = &wfx;

		LPDIRECTSOUNDCAPTUREBUFFER  pDSBPrimary = NULL;

		hr = capture_direct_sound_->CreateCaptureBuffer( &dscbd, &pDSBPrimary, NULL );
		if(FAILED( hr ) || !pDSBPrimary)
		{
			return false;
		}


		if(alloc_buffer)
		{
			if(capture_direct_sound_buf_)
			{
				capture_direct_sound_buf_->Release();
			}
			capture_direct_sound_buf_ = pDSBPrimary;
		}
		else
		{
			if(pDSBPrimary)
			{
				pDSBPrimary->Release();
			}
		}
		return true;
	}

	bool WindowsAudioDSound::TrySetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels, bool alloc_buffer )
	{
		if(!initialize_)
		{
			return false;
		}
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
		if(FAILED( hr ))
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

		int frameSize = wfx.nSamplesPerSec / 100 * wfx.nChannels * 2;
		int           framedelay = ( std::max )( 5UL, 1000 * frameSize / wfx.nAvgBytesPerSec );
		unsigned long dwBufferSize = ( std::max )( 2, ( MAX_DELAY / framedelay ) ) * frameSize + frameSize;


		hr = pDSBPrimary->SetFormat( &wfx );
		if(FAILED( hr ))
		{
			return false;
		}
		if(pDSBPrimary)
		{
			pDSBPrimary->Release();
			pDSBPrimary = NULL;
		}
		if(!alloc_buffer)
		{
			return true;
		}


		if(render_direct_sound_buf_)
		{
			render_direct_sound_buf_->Release();
		}
		dscbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2
			| DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS;

		dscbd.dwBufferBytes = dwBufferSize;
		dscbd.dwReserved = 0;
		dscbd.lpwfxFormat = &wfx;
		hr = render_direct_sound_->CreateSoundBuffer( &dscbd, &render_direct_sound_buf_, NULL );
		if(FAILED( hr ))
		{
			dscbd.dwFlags &= ~DSBCAPS_LOCHARDWARE;
			dscbd.dwFlags |= DSBCAPS_LOCSOFTWARE;
			hr = render_direct_sound_->CreateSoundBuffer( &dscbd, &render_direct_sound_buf_, NULL );

			if(FAILED( hr ))
				return false;
		}

		render_sample_rate_ = wfx.nSamplesPerSec;
		render_channel_ = wfx.nChannels;
		return true;
	}

	DWORD WindowsAudioDSound::DoCaptureThread()
	{
		if(!initialize_)
		{
			return 0;
		}

		HRESULT hr = S_OK;

		WAVEFORMATEX wfx;
		wfx.cbSize = 0;
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = (WORD)capture_channel_;
		wfx.nSamplesPerSec = capture_sample_rate_;
		wfx.wBitsPerSample = 16;
		wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
		wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
		int           frameSize = capture_sample_rate_ / 100 * capture_channel_ * 2;
		int           framedelay = std::max<int>( 5, 1000 * frameSize / wfx.nAvgBytesPerSec );
		unsigned long dwBufferSize = std::max<unsigned long>( 2, ( MAX_DELAY / framedelay ) ) * frameSize + frameSize;

		hr = capture_direct_sound_buf_->Start( DSCBSTART_LOOPING );
		if(FAILED( hr ))
		{
			goto err;
		}

		// capture data
		{
			long          nNextOffset = 0;
			unsigned long dwCurDevPos = 0;
			unsigned long dwCurDataPos = 0;
			SetEvent( wait_recording_thread_start_handle_ );
			while(recording_)
			{
				if(FAILED( hr = capture_direct_sound_buf_->GetCurrentPosition( &dwCurDevPos, &dwCurDataPos ) ))
					break;

				LONG datasize = dwCurDataPos - nNextOffset;
				if(datasize < 0)
					datasize += dwBufferSize;

				while(datasize > frameSize)
				{
					void*         pbCaptureData = NULL;
					unsigned long dwLockedSize = 0;
					hr = capture_direct_sound_buf_->Lock( nNextOffset, frameSize,
						&pbCaptureData, &dwLockedSize,
						NULL, NULL, 0L );

					if(FAILED( hr ))
						break;

					if(audio_buffer_proc_)
					{
						audio_buffer_proc_->RecordingDataIsAvailable( pbCaptureData, dwLockedSize );
						printf( "[%I64u]RecordingDataIsAvailable:%d\n", audio_engine::GetTickCount(), dwLockedSize );
					}

					capture_direct_sound_buf_->Unlock( pbCaptureData, dwLockedSize, NULL, NULL );
					datasize -= dwLockedSize;
					{
						LONG delayLength = dwCurDevPos - nNextOffset;
						if(delayLength < 0)
							delayLength += dwBufferSize;
					}

					nNextOffset += frameSize;
					if(nNextOffset == (long)dwBufferSize)
						nNextOffset = 0;
				}
				std::this_thread::sleep_for( std::chrono::milliseconds( framedelay / 2 ) );
				//Sleep( framedelay / 2 );
			}
		}
		capture_direct_sound_buf_->Stop();

	err:
		if(capture_direct_sound_buf_)
		{
			capture_direct_sound_buf_->Release();
			capture_direct_sound_buf_ = nullptr;
		}

		return 0;
	}

	DWORD WINAPI WindowsAudioDSound::WSAPIRenderThread( LPVOID context )
	{
		return reinterpret_cast<WindowsAudioDSound*>( context )->
			DoRenderThread();
	}
}