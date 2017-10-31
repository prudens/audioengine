#pragma once

#include "device/include/audio_device.h"
#include "windows_audio_device_helper.h"
#include <mutex>
#include <dsound.h>
#include <atomic>
namespace audio_engine
{
	class WindowsAudioDSound : public AudioDevice
	{
		struct DEVICE_INFO
		{
			GUID guid;
			char name[256];
		};
		typedef std::vector<DEVICE_INFO> DEVICE_INFO_LIST;
		enum {
			MAX_DELAY = 120,
			MIN_DELAY = 40UL,
			SAFE_DELAY = 40,
		};
	public:
		WindowsAudioDSound();
		virtual ~WindowsAudioDSound();
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
		bool TrySetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels, bool alloc_buffer );
		bool TrySetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels, bool alloc_buffer );
		static BOOL CALLBACK EnumCaptrueCallback( GUID* pGUID, LPCSTR szDesc, LPCSTR szDrvName, void* pContext );
		DWORD  DoRenderThread();
		DWORD DoCaptureThread();
		static DWORD WINAPI WSAPIRenderThread( LPVOID context );
		static DWORD WINAPI WSAPICaptureThread( LPVOID context );
	private:
		AUDIO_DEVICE_INFO_LIST capture_devices_;
		AUDIO_DEVICE_INFO_LIST render_devices_;
		bool initialize_ = false;
		std::mutex capture_lock_;
		std::mutex render_lock_;

		int capture_device_index_ = -1;
		int render_device_index_ = -1; //default

		int capture_sample_rate_ = 16000;
		int render_sample_rate_ = 16000;
		int capture_channel_ = 1;
		int render_channel_ = 1;

		LPDIRECTSOUNDCAPTURE capture_direct_sound_ = nullptr;
		LPDIRECTSOUND render_direct_sound_ = nullptr;
		LPDIRECTSOUNDBUFFER  render_direct_sound_buf_ = nullptr;
		LPDIRECTSOUNDCAPTUREBUFFER  capture_direct_sound_buf_ = nullptr;
		bool init_playout_ = false;
		bool init_recording_ = false;
		std::atomic<bool> playing_ = false;
		std::atomic<bool> recording_ = false;

		HANDLE     playout_thread_handle_;
		HANDLE     recording_thread_handle_;
		HANDLE     wait_playout_thread_start_handle_;
		HANDLE     wait_recording_thread_start_handle_;
		AudioBufferProc* audio_buffer_proc_;
	};
}