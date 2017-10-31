#pragma once
#include "device/include/audio_device.h"
#include "windows_audio_device_helper.h"
#include <mutex>
#include <dsound.h>
#include <atomic>
namespace audio_engine
{
	class WindowsAudioWave : public AudioDevice
	{
		enum{
			N_BUFFERS_OUT = 100,
			N_BUFFERS_IN = 100,
		};
	public:

		WindowsAudioWave();
		~WindowsAudioWave();
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
		void GetCaptureDeviceList();
		void GetRenderDeviceList();
		HWAVEOUT OpenRenderDevice();
		HWAVEIN  OpenCaptureDevice();
		void TraceWaveInError( MMRESULT error ) const;
		void TraceWaveOutError( MMRESULT error ) const;

		DWORD  DoRenderThread();
		DWORD DoCaptureThread();
		static DWORD WINAPI WSAPIRenderThread( LPVOID context );
		static DWORD WINAPI WSAPICaptureThread( LPVOID context );
		AUDIO_DEVICE_INFO_LIST capture_devices_;
		AUDIO_DEVICE_INFO_LIST render_devices_;
		int capture_device_index_ = 0;
		int render_device_index_ = 0; //default
		bool initialize_ = false;
		std::atomic<bool> playing_ = false;
		std::atomic<bool> recording_ = false;

		HANDLE     playout_thread_handle_ = nullptr;
		HANDLE     recording_thread_handle_ = nullptr;
		HANDLE     wait_playout_thread_start_handle_ = nullptr;
		HANDLE     wait_recording_thread_start_handle_ = nullptr;

		int capture_sample_rate_ = 16000;
		int render_sample_rate_ = 16000;
		uint16_t capture_channel_ = 1;
		uint16_t render_channel_ = 1;

		AudioBufferProc* audio_buffer_proc_ = nullptr;
	};
}