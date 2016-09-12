#include "audio_mixer_impl.h"



AudioMixerImpl::AudioMixerImpl( int samplerate, int channel )
{

}

AudioMixerImpl::~AudioMixerImpl()
{

}

void AudioMixerImpl::Release()
{
    delete this;
}

void AudioMixerImpl::AddParticipant( MixerParticipant *participant )
{
    for (const auto &v : m_participants)
    {
        if ( v.participant == participant )
            return;
    }

    MixerParticipantInfo info;
    info.participant = participant;
    m_participants.push_back( info );
}

void AudioMixerImpl::RemoveParticipant( MixerParticipant *participant )
{
    m_participants.remove_if( [=] (MixerParticipantInfo& v)
    {
        return ( v.participant == participant );
    } );
}

bool AudioMixerImpl::GetMixerAudio( webrtc::AudioFrame* audioFrame )
{
    if (m_participants.empty())
    {
        return false;
    }
    // 分类
    bool bMix = false;
    for ( auto& v : m_participants )
    {
        auto participant = v.participant;
        auto audioFrame = PopAudioFrame();
        bool ret = participant->GetAudioFrame( audioFrame );
        if ( !ret )
        {
            v.needMixCount = 0;
            v.weightFactor = 0.0f;
            v.audioFrame = nullptr;
            v.energy = 0;
            v.isMixed = false;
            PushAudioFrame( audioFrame );
            continue;
        }
        bMix = true;
        v.energy = CalculateEnergy( audioFrame );
        v.isSilent = audioFrame->vad_activity_ != webrtc::AudioFrame::kVadActive;
        v.audioFrame = audioFrame;
    }
    if (!bMix)
    {
        return false;
    }
    if (m_participants.size() > (uint32_t)m_limitCount)
    {
        m_participants.sort( [] ( MixerParticipantInfo& lhs, MixerParticipantInfo& rhs )
        {
            return (lhs.energy > rhs.energy);
        } );
    }

    MixerParticipantList mixerlist;
    int nMixframeCounter = 0;
    for ( auto it = m_participants.begin(); it != m_participants.end(); ++it )
    {
        if ( m_limitCount >= nMixframeCounter )
        {
            if (it->energy > 0)
            {
                nMixframeCounter++;
                it->needMixCount = 5;
                it->isMixed = true;
                // 这里系数的调整至关重要，目前先简单的处理下
                if (it->isSilent)
                {
                    it->weightFactor = 0.7f;
                }
                else
                {
                    it->weightFactor = 1.0f;
                }
                mixerlist.push_back(*it);
            }
        }
        else
        {
            if (it->isMixed)
            {
                if ( it->needMixCount-- > 0 )
                    it->weightFactor /= 2; // 渐渐淡出

                mixerlist.push_back( *it );
            }
        }
    }

    MixFrameList( mixerlist, audioFrame );
    for ( auto&v : m_participants )
    {
        if ( v.audioFrame )
            PushAudioFrame( v.audioFrame );
    }
    return true;
}

void AudioMixerImpl::LimitParticipantCount( int32_t count )
{
    m_limitCount = count;
}

webrtc::AudioFrame* AudioMixerImpl::PopAudioFrame()
{
    if (m_audioFramePool.empty())
    {
        return new webrtc::AudioFrame;
    }
    auto pAudioFrame = m_audioFramePool.front();
    m_audioFramePool.pop_front();
    return pAudioFrame;
}

void AudioMixerImpl::PushAudioFrame( webrtc::AudioFrame* pAudioFrame )
{
    m_audioFramePool.push_back( pAudioFrame );
}

uint32_t  AudioMixerImpl::CalculateEnergy( const webrtc::AudioFrame* audioFrame )
{
    uint32_t energy = 0;
    for ( size_t position = 0; position < audioFrame->samples_per_channel_*audioFrame->num_channels_;
          position++ )
    {
        energy += audioFrame->data_[position] * audioFrame->data_[position];
    }
    return energy;
}

void AudioMixerImpl::MixFrameList( MixerParticipantList & mixlist, webrtc::AudioFrame* audioFrame )
{
    if ( mixlist.empty() )
    {
        memset( audioFrame->data_, 0, audioFrame->samples_per_channel_*audioFrame->num_channels_ * 2 );//静音包
        return;
    }
    size_t size = audioFrame->samples_per_channel_*audioFrame->num_channels_;
    float* data = new float[size];
    memset( data, 0, sizeof( float )*size );
    for ( auto& v : mixlist )
    {
        for ( size_t i = 0; i < size; i++ )
        {
            data[i] += v.weightFactor * v.audioFrame->data_[i];
        }
    }
    size_t overflow = std::count_if( data, data + size, [] (int32_t v)
    {
        return ( v > 32767 || v < -32768 );
    } );
    float scale = 1.0f;
    if (overflow > size/20)
    {
        scale = 0.8f;
    }
    else if ( overflow > size / 40)
    {
        scale = 0.9f;
    }
    for ( size_t i = 0; i < size; i++ )
    {
        int32_t v = static_cast<int32_t>(scale * data[i]);
        if ( v > 32767 )
        {
            audioFrame->data_[i] = 32767;
        }
        else if ( data[i] < -32768)
        {
            audioFrame->data_[i] = -32768;
        }
        else
        {
            audioFrame->data_[i] = v;
        }
    }
}

AudioMixer* AudioMixer::Create(int samplerate,int channel)
{
    return new AudioMixerImpl( samplerate, channel );
}