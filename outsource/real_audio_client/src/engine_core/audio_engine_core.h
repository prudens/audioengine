#pragma once
#include <set>
#include <memory>
#include <mutex>
#include "device/include/audio_device.h"
#include "audio_processing.h"
#include "audio_mixer.h"
#include "audio_neteq.h"
#include "codec_converter.h"
#include "base/time_cvt.hpp"
#include "audio_resample.h"
#include "audio_buffer.h"
#include "silent_voice_filter.h"
namespace audio_engine
{


	class IAudioDataCallback
	{
	public:
		virtual void CaptureData( AudioBufferPtr buffer ){}//原始数据
		virtual bool IsNeedProcess(){ return false; }//是否需要预处理
		virtual void ProcessData( AudioBufferPtr buffer ){}//前端预处理，包括aec，ns，vad等等
		virtual bool IsNeedSilentFilter (){ return true; }//是否需要过滤静音包
		virtual bool IsNeedDefaultEncode(){ return false; }//是否需要默认编码器，opus
		virtual void BeforeEncodeData( AudioBufferPtr buffer ){}//未编码数据，有特殊需要的可自行处理。
		virtual void PostEncodeData(AudioBufferPtr buffer){}//默认编码的数据
		virtual bool DecodeData( AudioBufferPtr buffer ){ return false; }//解码前
		virtual void AfterDecodeData( AudioBufferPtr buffer ){ }//解码后
		virtual void MixerData( AudioBufferPtr buffer ){}//混音后
		virtual void PlayoutData( AudioBufferPtr buffer ){}//送到设备播放的数据
	};

	class AudioEngineCore:public AudioBufferProc
	{
	public:
		AudioEngineCore();
		~AudioEngineCore();
		void Initialize();
		void Terminate();
		void StartRecording();
		void StartPlayout();
		void StopRecording();
		void StopPlayout();
		void AddObserver( IAudioDataCallback* cb );
		void RemoveObserver( IAudioDataCallback* cb );
	protected:
		virtual void RecordingDataIsAvailable( const void* data, size_t size_in_byte );
		virtual size_t NeedMorePlayoutData( void* data, size_t size_in_byte );
		virtual void ErrorOccurred( AudioError /*aeCode*/ );
	private:
		AudioBufferPtr PreProcess(const void* data, size_t size_in_byte);
		void ProcessEncode();
		class AudioDataCallbackProxy : public IAudioDataCallback
		{
		public:
			virtual void CaptureData( AudioBufferPtr buffer );
			virtual bool IsNeedProcess();
			virtual void ProcessData( AudioBufferPtr buffer );
			virtual bool IsNeedSilentFilter();
			virtual bool IsNeedDefaultEncode();
			virtual void BeforeEncodeData( AudioBufferPtr buffer );
			virtual void PostEncodeData( AudioBufferPtr buffer );
			virtual bool DecodeData( AudioBufferPtr buffer );
			virtual void AfterDecodeData( AudioBufferPtr buffer );
			virtual void MixerData( AudioBufferPtr buffer );
			virtual void PlayoutData( AudioBufferPtr buffer );
			void AddObserver( IAudioDataCallback* cb );
			void RemoveObserver( IAudioDataCallback* cb );
		private:
			std::mutex _mutex;
			std::set<IAudioDataCallback*> _audio_data_cb_list;
		};
	private:
		AudioDevice* _audio_device;
		AudioProcess _audio_process;
		AudioMixer   _audio_mixer;
		AudioNetEq   _audio_neteq;
		CodecFactory _codec_factory;
		AudioDataCallbackProxy _cb_proxy;
		AudioResample _rec_resampler;
		SilentVoiceFilter _silent_voice_filter;
		CodecConverter* _encoder;
	};
}