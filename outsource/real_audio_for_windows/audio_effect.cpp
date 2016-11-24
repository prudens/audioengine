#include "audio_effect.h"
#include <string>
#include <chrono>
#include "webrtc/modules/include/module_common_types.h"
#include "audio_resample.h"


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
    m_apm->echo_cancellation()->set_suppression_level( EchoCancellation::kHighSuppression );
    m_apm->echo_control_mobile()->Enable( false );
    m_apm->echo_control_mobile()->set_routing_mode( EchoControlMobile::kSpeakerphone );
    m_apm->echo_control_mobile()->enable_comfort_noise( false );


    m_apm->gain_control()->Enable( false );
    m_apm->gain_control()->set_target_level_dbfs( 9 );
    m_apm->gain_control()->set_mode( GainControl::kAdaptiveDigital );
    m_apm->gain_control()->set_compression_gain_db( 90 );
    m_apm->voice_detection()->Enable( false );
    m_apm->noise_suppression()->Enable( false );
    m_apm->high_pass_filter()->Enable( false );

    m_apm->noise_suppression()->set_level( webrtc::NoiseSuppression::kVeryHigh );
    m_apm->voice_detection()->set_likelihood( VoiceDetection::kLowLikelihood );
    m_apm->level_estimator()->Enable( false );
	 
    m_bEnable = true;
    m_nCheckVad = 0;
    m_stream_delay = kAecTotalDelayMs;
    m_bInit = true;
    m_nNormalVoice = 0;
    m_bSilent = false;

}

AudioEffect::~AudioEffect()
{
    delete m_apm;
}

void AudioEffect::RecordingReset( size_t inFreq, size_t inChannel, size_t outFreq, size_t outChannel )
{
    rec_resample.infreq = inFreq;
    rec_resample.outfreq = outFreq;
    rec_resample.inchannel = inChannel;
    rec_resample.outchannel = outChannel;
    rec_resample.channel = (std::min)( inChannel, outChannel );
    rec_resample.frame_size = inFreq * inChannel * 2 / 100;
    m_recResample.ResetIfNeeded( rec_resample.infreq/1000*1000, kTargetRecSampleRate, rec_resample.channel );
    m_recReverseResample.ResetIfNeeded( kTargetRecSampleRate, rec_resample.outfreq/1000*1000, rec_resample.channel);
}


void AudioEffect::PlayoutReset( size_t inFreq, size_t inChannel, size_t outFreq, size_t outChannel )
{
    ply_resample.infreq = inFreq;
    ply_resample.outfreq = outFreq;
    ply_resample.inchannel = inChannel;
    ply_resample.outchannel = outChannel;
    ply_resample.channel = ( std::min )( inChannel, outChannel );
    ply_resample.frame_size = inFreq * inChannel * 2 / 100;
    m_plyResample.ResetIfNeeded( ply_resample.infreq/1000*1000, kTargetPlySampleRate, ply_resample.channel );
    m_plyReverseResample.ResetIfNeeded( ply_resample.infreq/1000*1000, ply_resample.outfreq/1000*1000, ply_resample.channel );
}

void AudioEffect::ProcessCaptureStream( int16_t* audio_samples, size_t frame_byte_size, int16_t*outSample, size_t& len_of_byte )
{
    if ( !m_bInit )
    {
        return;
    }
    if ( !m_bEnable )
    {
        return;
    }

    if ( rec_resample.infreq == 44100)
    {
        frame_byte_size = 880 * rec_resample.inchannel;
    }

    if (rec_resample.inchannel == 2 && rec_resample.channel == 1)
    {
        AudioResample::ToMono( audio_samples, frame_byte_size/2 );
        frame_byte_size /= 2;
    }

    AudioFrame af;
    size_t outLen = 0;
    int err = 0;
    if ( 0 != (err = m_recResample.Push( audio_samples,
                                         frame_byte_size/sizeof(int16_t), 
                                         af.data_, 
                                         sizeof( af.data_ ), 
                                         outLen ) ) )
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
                    rec_resample.channel );
    m_apm->set_stream_delay_ms( m_stream_delay );
    if ( 0 != (err = m_apm->ProcessStream( &af )) )
    {
       return;
    }


    size_t inLen = outLen;
    if ( 0 != ( err = m_recReverseResample.Push( af.data_,
        inLen,
        outSample,
        len_of_byte/2,
        outLen ) ) )
    {
        return;
    }

    if (rec_resample.outchannel == 2 && rec_resample.channel == 1)
    {
        AudioResample::Tostereo( outSample, outLen);
        outLen *= 2;
    }

    if ( rec_resample.outfreq == 44100 )
    {
        if ( rec_resample.outchannel == 1 )
        {
            outSample[440] = outSample[439];
        }
        else
        {
            outSample[880] = outSample[878];
            outSample[880 + 1] = outSample[879];
        }
    }

    len_of_byte = rec_resample.outfreq / 100 * rec_resample.outchannel * 2;
}

void AudioEffect::ProcessRenderStream( int16_t*  inSamples, size_t frame_byte_size,int16_t*outSample, size_t& len_of_byte )
{
    if ( !m_bInit )
    {
        return;
    }
    if ( !m_bEnable )
    {
        return;
    }
    if ( ply_resample.infreq == 44100 )
    {
        frame_byte_size = 880 * ply_resample.inchannel;
    }
    if ( ply_resample.inchannel == 2 && ply_resample.channel == 1 )
    {
        AudioResample::ToMono( inSamples, frame_byte_size / 2 );
        frame_byte_size /= 2;
    }

    size_t outLen;
    int err =0;
    AudioFrame af;
    if ( 0 != (err = m_plyResample.Push( inSamples,
                                         frame_byte_size/sizeof(int16_t),
                                         af.data_,
                                         sizeof( af.data_ ),
                                         outLen ) ))
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
                    ply_resample.channel );

    if ( 0 != (err = m_apm->AnalyzeReverseStream( &af ) ) )
    {
        return;
    }
    if (ply_resample.infreq == ply_resample.outfreq)
    {
        if (ply_resample.inchannel == 1 && ply_resample.outchannel == 2)
        {
            AudioResample::Tostereo( inSamples, frame_byte_size/2, outSample );
        }
        else if ( inSamples != outSample )
        {
            memcpy( outSample, inSamples, len_of_byte );
        }
        len_of_byte = ply_resample.outfreq / 100 * 2 * ply_resample.outchannel;
        return;
    }

    size_t inLen = outLen;
    if (ply_resample.infreq != ply_resample.outfreq || inSamples != outSample)
    {
        if ( 0 != ( err = m_plyReverseResample.Push( inSamples,
            frame_byte_size / 2,
            outSample,
            1920,
            outLen ) ) )
        {
            return;
        }
    }

    if ( ply_resample.inchannel == 1 && ply_resample.outchannel == 2)
    {
        AudioResample::Tostereo( outSample, outLen );
    }

    if ( ply_resample.outfreq == 44100 )
    {
        if ( ply_resample.outchannel == 1 )
        {
            outSample[440] = outSample[439];
        }
        else
        {
            outSample[880] = outSample[878];
            outSample[880 + 1] = outSample[879];
        }
    }

    len_of_byte = ply_resample.outfreq / 100 * 2 * ply_resample.outchannel;
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
	if(m_apm->voice_detection()->is_enabled())
		return m_apm->voice_detection()->stream_has_voice(); // 1s
	else
		return true;
}

bool AudioEffect::HadProcessingVoice()
{
    // 检测是否有语音进来。
    return m_nNormalVoice > 50;
}

