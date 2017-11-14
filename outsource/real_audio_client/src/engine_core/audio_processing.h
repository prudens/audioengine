#pragma once
#include "audio_buffer.h"
#include "webrtc/modules/audio_processing/include/audio_processing.h"
namespace audio_engine{
	class AudioProcess
	{
	public:
		AudioProcess();
		~AudioProcess();
		bool ProcessCaptureStream(AudioBufferPtr buf );
		void ProcessRenderStream( AudioBufferPtr buf );//for aec
		void EnableAec( bool bEnable );
		void EnableAecm( bool bEnable );
		void EnableVad( bool bEnable );
		void EnableHighPassFilter( bool bEnable );
		void EnableAgc( bool bEnable );
		void EnableLowPassFilter( bool bEnable );
		void EnableNs( bool bEnable );
		void EnableAudioEffect( bool bEnable );
		bool HasVoice()const;
		void SetAecDelay( tick_t delay );
	private:
		webrtc::AudioProcessing *m_apm;
		bool m_bEnable;
		int32_t m_stream_delay;
	};
}