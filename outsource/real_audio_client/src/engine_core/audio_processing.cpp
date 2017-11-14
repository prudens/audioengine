#include "audio_processing.h"
#include "webrtc\modules\include\module_common_types.h"
namespace audio_engine{

	AudioProcess::AudioProcess()
	{
		m_apm = webrtc::AudioProcessing::Create();
		m_apm->echo_cancellation()->Enable( true );
		m_apm->echo_control_mobile()->Enable( false );
		m_apm->echo_control_mobile()->set_routing_mode( webrtc::EchoControlMobile::kSpeakerphone );
		m_apm->echo_control_mobile()->enable_comfort_noise( false );


		m_apm->gain_control()->Enable( false );
		m_apm->gain_control()->set_target_level_dbfs( 9 );
		m_apm->gain_control()->set_mode( webrtc::GainControl::kAdaptiveDigital );
		m_apm->gain_control()->set_compression_gain_db( 90 );
		m_apm->voice_detection()->Enable( true );
		m_apm->noise_suppression()->Enable( true );
		m_apm->high_pass_filter()->Enable( true );

		m_apm->noise_suppression()->set_level( webrtc::NoiseSuppression::kVeryHigh );
		m_apm->voice_detection()->set_likelihood( webrtc::VoiceDetection::kLowLikelihood );
		m_apm->level_estimator()->Enable( true );
	}
	AudioProcess::~AudioProcess()
	{
		delete m_apm;
	}

	bool AudioProcess::ProcessCaptureStream( AudioBufferPtr buf )
	{
		if(!m_bEnable)
		{
			return false;
		}
		webrtc::AudioFrame af;
		af.UpdateFrame( buf->id,
			buf->ts.count(),
			buf->data,
			160,
			16000,
			webrtc::AudioFrame::kNormalSpeech,
			webrtc::AudioFrame::kVadUnknown,
			buf->channel );
		m_apm->set_stream_delay_ms( m_stream_delay );
		int err = m_apm->ProcessStream( &af );
		if(err)
		{
			return false;
		}
		memcpy( buf->data, af.data_, buf->nsamples * 2 );
		buf->rms = m_apm->level_estimator()->RMS();
		buf->silent = !m_apm->voice_detection()->stream_has_voice();
		return true;
	}

	void AudioProcess::ProcessRenderStream( AudioBufferPtr buf )
	{
		if(!m_bEnable)
		{
			return;
		}

		int err = 0;
		webrtc::AudioFrame af;
		af.UpdateFrame( 0,
			buf->ts.count(),
			buf->data,
			160,
			16000,
			webrtc::AudioFrame::kNormalSpeech,
			webrtc::AudioFrame::kVadUnknown,
			buf->channel );

		err = m_apm->AnalyzeReverseStream( &af );
	}

	void AudioProcess::EnableAec( bool bEnable )
	{
		m_apm->echo_cancellation()->Enable( bEnable );
		m_apm->echo_control_mobile()->Enable( !bEnable );
	}

	void AudioProcess::EnableAecm( bool bEnable )
	{
		m_apm->echo_control_mobile()->Enable( bEnable );
		m_apm->echo_cancellation()->Enable( !bEnable );
	}

	void AudioProcess::EnableVad( bool bEnable )
	{
		m_apm->voice_detection()->Enable( bEnable );
	}

	void AudioProcess::EnableHighPassFilter( bool bEnable )
	{
		m_apm->high_pass_filter()->Enable( bEnable );
	}

	void AudioProcess::EnableAgc( bool bEnable )
	{
		m_apm->gain_control()->Enable( bEnable );
	}

	void AudioProcess::EnableLowPassFilter( bool bEnable )
	{
	}

	void AudioProcess::EnableNs( bool bEnable )
	{
		m_apm->noise_suppression()->Enable( bEnable );
	}

	void AudioProcess::EnableAudioEffect( bool bEnable )
	{
		m_bEnable = bEnable;
	}

	bool AudioProcess::HasVoice() const
	{
		if(m_apm->voice_detection()->is_enabled())
			return m_apm->voice_detection()->stream_has_voice(); // 1s
		else
			return true;
	}

	void AudioProcess::SetAecDelay( tick_t delay )
	{
		m_stream_delay = delay.count();
	}

}