
#include "audio_effect.h"
#include <string>
#include <chrono>
#include "webrtc\modules\include\module_common_types.h"

static uint32_t GetTimeStamp()
{
    using namespace std::chrono;
    return static_cast<uint32_t>( duration_cast<milliseconds>(
        system_clock::now() - time_point<system_clock>() ).count());
}
AudioEffect::AudioEffect()
{
    m_apm = AudioProcessing::Create();
    m_apm->echo_cancellation()->Enable( false );
    m_apm->echo_control_mobile()->Enable( false );
    m_apm->echo_control_mobile()->set_routing_mode( EchoControlMobile::kSpeakerphone );
    m_apm->echo_control_mobile()->enable_comfort_noise( false );


    m_apm->gain_control()->Enable( false );
    m_apm->gain_control()->set_target_level_dbfs( 9 );
    m_apm->gain_control()->set_mode( GainControl::kAdaptiveDigital );
    m_apm->gain_control()->set_compression_gain_db( 90 );
    m_apm->voice_detection()->Enable( true );
    m_apm->noise_suppression()->Enable( true );
    m_apm->high_pass_filter()->Enable( false );

    m_apm->noise_suppression()->set_level( webrtc::NoiseSuppression::kLow );
    m_apm->voice_detection()->set_likelihood( VoiceDetection::kVeryLowLikelihood );
    m_apm->level_estimator()->Enable( true );
    m_bEnable = true;
    m_nCheckVad = 0;
    m_stream_delay = kAecTotalDelayMs;
    m_bInit = false;
    m_nNormalVoice = 0;
}

AudioEffect::~AudioEffect()
{
    delete m_apm;
}

void AudioEffect::Init( size_t recSampleRate, size_t recChannel, size_t plySampleRate, size_t plyChannel )
{
    m_recSampleRate = recSampleRate / 1000 * 1000;
    m_recChannel = recChannel;
    m_plySampleRate = plySampleRate / 1000 * 1000;
    m_plyChannel = plyChannel;
    m_recResample.ResetIfNeeded( m_recSampleRate, kTargetRecSampleRate, m_recChannel );
    m_plyResample.ResetIfNeeded( m_plySampleRate, kTargetPlySampleRate, m_plyChannel );
    m_recReverseResample.ResetIfNeeded( kTargetRecSampleRate, m_recSampleRate, m_recChannel );
    m_plyReverseResample.ResetIfNeeded( kTargetPlySampleRate,m_plySampleRate, m_plyChannel );
//    ParseParamNotify(std::string());
    m_bInit = true;
}

void AudioEffect::ProcessCaptureStream( int16_t* audio_samples, size_t frame_byte_size )
{
    if ( !m_bInit )
    {
        return;
    }
    if ( !m_bEnable )
    {
        return;
    }

    bool b441 = false;
    if ( frame_byte_size / 2 / m_recChannel == 441 )
    {
        frame_byte_size = 440 * m_recChannel * 2;
        b441 = true;
    }
    AudioFrame af;
    size_t outLen = 0;
    int err = 0;
    if ( 0 != (err = m_recResample.Push( audio_samples, frame_byte_size/sizeof(int16_t), af.data_, sizeof( af.data_ ), outLen ) ) )
    {
        return;
    }

    af.UpdateFrame( 0,
                    GetTimeStamp(),
                    af.data_,
                    kTargetRecSampleRate/100,
                    kTargetRecSampleRate,
                    AudioFrame::kNormalSpeech,
                    AudioFrame::kVadUnknown,
                    m_recChannel );
    m_apm->set_stream_delay_ms( m_stream_delay );
    if ( 0 != (err = m_apm->ProcessStream( &af )) )
    {
        return;
    }

    if ( 0 != (err = m_recReverseResample.Push( af.data_, outLen, audio_samples, frame_byte_size / sizeof( int16_t ), outLen ) ) )
    {
        return;
    }
    
    m_rms = m_apm->level_estimator()->RMS();
    if (!m_apm->voice_detection()->stream_has_voice())
    {
        memset( audio_samples, 0, frame_byte_size );
    }

    if ( b441 )
    {
        if ( m_recChannel == 1 )
        {
            audio_samples[440] = audio_samples[439];
        }
        else
        {
            audio_samples[880] = audio_samples[878];
            audio_samples[880 + 1] = audio_samples[879];
        }
    }

    if (false)
    {
        
        m_nCheckVad++;
    }
    else
    {
        m_nNormalVoice++;
        m_nCheckVad = 0;
    }
}

void AudioEffect::ProcessRenderStream( int16_t*  audio_samples, size_t frame_byte_size )
{
    if ( !m_bInit )
    {
        return;
    }
    if ( !m_bEnable )
    {
        return;
    }

    if ( frame_byte_size / 2 / m_recChannel == 441 )
    {
        frame_byte_size = 440 * m_recChannel * 2;
    }

    size_t outLen;
    int err =0;
    AudioFrame af;
    if ( 0 != (err = m_plyResample.Push( audio_samples, frame_byte_size/sizeof(int16_t), af.data_, sizeof( af.data_ ), outLen ) ))
    {
        return;
    }
    af.UpdateFrame( 0, 
                    GetTimeStamp(),
                    af.data_,
                    kTargetPlySampleRate/100,
                    kTargetPlySampleRate,
                    AudioFrame::kNormalSpeech,
                    AudioFrame::kVadUnknown,
                    m_plyChannel );

    if ( 0 != (err = m_apm->AnalyzeReverseStream( &af ) ) )
    {
        return;
    }
}

void AudioEffect::EnableAec( bool bEnable )
{
    m_apm->echo_cancellation()->Enable( bEnable );
    m_apm->echo_control_mobile()->Enable( !bEnable );
}

void AudioEffect::EnableAecm( bool bEnable )
{
    m_apm->echo_control_mobile()->Enable( bEnable );
    m_apm->echo_cancellation()->Enable( !bEnable );
}

void AudioEffect::EnableVad( bool bEnable )
{
    m_apm->voice_detection()->Enable( bEnable );
}

void AudioEffect::EnableHighPassFilter( bool bEnable )
{
    m_apm->high_pass_filter()->Enable( bEnable );
}

void AudioEffect::EnableAgc( bool bEnable )
{
    m_apm->gain_control()->Enable( bEnable );
}

void AudioEffect::EnableLowPassFilter( bool bEnable )
{
}

void AudioEffect::EnableNs( bool bEnable )
{
    m_apm->noise_suppression()->Enable( bEnable );
}

void AudioEffect::EnableAudioEffect( bool bEnable )
{
    m_bEnable = bEnable;
}

bool AudioEffect::HasVoice() const
{
    return m_nCheckVad < 10; // 1s
}

bool AudioEffect::HadProcessingVoice()
{
    // 检测是否有语音进来。
    return m_nNormalVoice > 50;
}

void AudioEffect::ParseParamNotify( const std::string& /*Param*/ )
{
}

int AudioEffect::GetRMS()
{
    return m_rms;
}

