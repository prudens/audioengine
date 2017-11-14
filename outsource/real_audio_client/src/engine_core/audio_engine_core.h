#pragma once
#include <set>
#include <memory>
#include "device/include/audio_device.h"
#include "audio_processing.h"
#include "audio_mixer.h"
#include "audio_neteq.h"
#include "codec_conveter.h"
#include "base/time_cvt.hpp"
#include "audio_resample.h"
#include "audio_buffer.h"
namespace audio_engine
{


	class IAudioDataCallback
	{
	public:
		virtual void CaptureData( AudioBufferPtr buffer ){}//原始数据
		virtual bool IsNeedProcess(){ return false; }//是否需要预处理
		virtual void ProcessData( AudioBufferPtr buffer ){}//前端预处理，包括aec，ns，vad等等
		virtual bool IsNeedDefaultEncode(){ return false; }//是否需要默认编码器，opus
		virtual void BeforeEncodeData( AudioBufferPtr buffer ){}//未编码数据，有特殊需要的可自行处理。
		virtual void PostEncodeData(AudioBufferPtr buffer){}//默认编码的数据
		virtual bool BeforeDecodeData(){ return false; }//解码前
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
		void SetAudioDataCallback( IAudioDataCallback* cb );
		void RemoveAudioDataCallback( IAudioDataCallback* cb );
	protected:
		virtual void RecordingDataIsAvailable( const void* data, size_t size_in_byte );
		virtual size_t NeedMorePlayoutData( void* data, size_t size_in_byte );
		virtual void ErrorOccurred( AudioError /*aeCode*/ );
	private:
		AudioBufferPtr PreProcess(const void* data, size_t size_in_byte);
		void ProcessEncode();
	private:
		AudioDevice* _audio_device;
		AudioProcess _audio_process;
		AudioMixer   _audio_mixer;
		AudioNetEq   _audio_neteq;
		CodecConveter* _codec_conveter;
		std::set<IAudioDataCallback*> _audio_data_cb_list;
		AudioResample _rec_resampler;
	};
}