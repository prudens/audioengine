#pragma once
#include <mutex>
#include <atlbase.h>
#include <ATLComCli.h>
#include <uuids.h>

#include "media_buffer.h"
#include "device/include/audio_device.h"
#include "windows_audio_device_helper.h"
#include <bitset>
namespace audio_engine
{
	class WindowsCoreAudio final :public AudioDevice
	{
		typedef std::lock_guard<std::mutex> lock_guard;
	public:
		WindowsCoreAudio();
		virtual ~WindowsCoreAudio();
		virtual void Release() override;
		virtual bool Initialize()override;
		virtual void Terminate()override;
		virtual size_t GetRecordingDeviceNum()const override;
		virtual size_t GetPlayoutDeviceNum()const override;

		virtual bool GetPlayoutDeviceName(
			int16_t index,
			wchar_t name[kAdmMaxDeviceNameSize],
			wchar_t guid[kAdmMaxGuidSize] )override;
		virtual bool GetRecordingDeviceName(
			int16_t index,
			wchar_t name[kAdmMaxDeviceNameSize],
			wchar_t guid[kAdmMaxGuidSize] )override;

		virtual bool SetPlayoutDevice( int16_t index )override;
		virtual bool SetRecordingDevice( int16_t index )override;

		virtual bool IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels )override;
		virtual bool IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels )override;

		virtual bool SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )override;
		virtual bool SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )override;
		virtual bool GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )override;
		virtual bool GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )override;



		virtual bool StartPlayout()override;
		virtual bool StopPlayout()override;
		virtual bool Playing() const override;
		virtual bool StartRecording()override;
		virtual bool StopRecording()override;
		virtual bool Recording() const override;

		virtual void SetAudioBufferCallback( AudioBufferProc* pCallback ) override;
		virtual AudioBufferProc* GetAudioBufferCallback()const override;

		virtual bool SetPropertie( AudioPropertyID id, void* );
		virtual bool GetProperty( AudioPropertyID id, void* );
	private:
		bool    IsFormatSupported( CComPtr<IAudioClient> audioClient, DWORD nSampleRate, WORD nChannels );
		bool    SetDMOProperties();
		bool    InitRecordingMedia();
		bool    InitRecordingDMO();
		bool    InitPlayout();
		bool    InitRecording();
		DWORD   DoRenderThread();
		DWORD   DoCaptureThreadPollDMO();
		DWORD   DoCaptureThread();

		static DWORD WINAPI WSAPIRenderThread( LPVOID context );
		static DWORD WINAPI WSAPICaptureThreadPollDMO( LPVOID context );
		static DWORD WINAPI WSAPICaptureThread( LPVOID context );
	private:
		WindowsCoreAudio( const WindowsCoreAudio& ) = delete;
		WindowsCoreAudio( const WindowsCoreAudio&& ) = delete;
		WindowsCoreAudio& operator=( WindowsCoreAudio& ) = delete;
		WindowsCoreAudio& operator=( WindowsCoreAudio&& ) = delete;
	private:
		ScopedCOMInitializer                    com_init_;
		std::mutex                              audio_lock_;
		CComPtr<IMediaObject>                   dmo_;// DirectX Media Object (DMO) for the built-in AEC.
		CComPtr<IAudioClient>                   audio_client_in_;
		CComPtr<IAudioClient>                   audio_client_out_;
		CComPtr<IAudioRenderClient>             render_client_;
		CComPtr<IAudioCaptureClient>            capture_client_;
		CComPtr<IMediaBuffer>                   media_buffer_;

		AudioBufferProc*                        audio_buffer_proc_ = nullptr;

		size_t                                  capture_sample_rate_ = 0;
		size_t                                  render_sample_rate_ = 0;
		uint16_t                                capture_channel_ = 0;
		uint16_t                                render_channel_ = 0;
		int16_t                                 capture_device_index_ = 0;
		int16_t                                 render_device_index_ = 0;

		bool                                    use_dmo_ = true;
		bool                                    dmo_is_available_ = true;
		bool                                    initialize_ = false;
		bool                                    recording_ = false;
		bool                                    playing_ = false;


		HANDLE                                  render_samples_ready_evnet_ = nullptr;
		HANDLE                                  playout_thread_handle_ = nullptr;
		HANDLE                                  render_started_event_ = nullptr;
		HANDLE                                  shutdown_render_event_ = nullptr;

		HANDLE                                  capture_samples_ready_event_ = nullptr;
		HANDLE                                  recording_thread_handle_ = nullptr;
		HANDLE                                  capture_start_event_ = nullptr;
		HANDLE                                  shutdown_capture_event_ = nullptr;

		AUDIO_DEVICE_INFO_LIST                  capture_device_list_;
		AUDIO_DEVICE_INFO_LIST                  render_device_list_;

		std::bitset<sizeof( uint32_t )>           set_effect_;
	};
}