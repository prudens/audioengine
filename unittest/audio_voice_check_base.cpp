#include "audio_voice_check_base.h"
#include <map>
#include "audio_parse_param.h" 

namespace snail{
namespace audio{
		 
    AudioVoiceCheck::AudioVoiceCheck()
    {
    }

    AudioVoiceCheck::~AudioVoiceCheck()
    {
    }

    void AudioVoiceCheck::Reset( int frame_time )
    {
        if ( frame_time < 10 )
        {
            return;
        }

        int32_t value = 0;
        int send_end_span_ttl = 150;
        m_mute_level_ttl = 60;
        if ( AudioParseParameter::GetInstance().GetValue( "cs", 'c', value, 0, 2 ) )
        {
            m_enable = ( 0 != value );
        }
        if ( AudioParseParameter::GetInstance().GetValue( "spk", 'e', value, 0, 5000 ) )
        {
            send_end_span_ttl = value;
        }
        if ( AudioParseParameter::GetInstance().GetValue( "spk", 'l', value, 30, 125 ) )
        {
            m_mute_level_ttl = value;
        }

        m_sent_mute_ttl = send_end_span_ttl / frame_time;
        if ( m_sent_mute_ttl < 8 )
        {
            m_sent_mute_ttl = 8;
        }
        m_mute_cnt = 0;
        m_level_overflow_cnt = 0;
        m_cache_frames.clear();
        m_send_frames.clear();
        m_mute_level_gain = 0;
    }




    void AudioVoiceCheck::Enable( bool bEnable )
    {
        m_enable = bEnable;
        if ( !m_enable )
        {
            m_mute_cnt = 0;
        }
    }


    void AudioVoiceCheck::SetLevel( int level )
    {
        if ( level <= 30 || level > 125 )
        {
            return;
        }
        m_mute_level_ttl = level;
    }

    void AudioVoiceCheck::Process( const void* buf, int len, bool vadspk, int rms_level )
    {
        auto frame = std::make_shared<tFrame>( buf, len );
        if ( !m_enable )
        {
            m_send_frames.push_back( frame );
            return;
        }

        if ( m_mute_cnt < m_sent_mute_ttl - 4 )
        {
            m_send_frames.push_back( frame );
        }

        if ( !vadspk || rms_level >= m_mute_level_ttl )
        {
            m_mute_cnt++;
            m_level_overflow_cnt = 0;
            m_cache_frames.clear();
        }
        else
        {
            if ( m_mute_cnt >= m_sent_mute_ttl - 4)
            {
                m_mute_cnt++;
                m_level_overflow_cnt++;
                m_cache_frames.push_back( frame );
            }
            if ( m_level_overflow_cnt > CACHE_FRAME_NUM )
            {
                m_mute_cnt = 0;
                m_send_frames = m_cache_frames;
                m_cache_frames.clear();
                m_level_overflow_cnt = 0;
            }
        }
    }


    AudioVoiceCheck::frameptr AudioVoiceCheck::GetFrame()
    {
        if (m_send_frames.empty())
        {
            return nullptr;
        }
        auto p = m_send_frames.front();
        m_send_frames.pop_front();
        return p;
    }

    bool AudioVoiceCheck::IsEnable()
    {
        return m_enable;
    }

    void AudioVoiceCheck::UpdateSpeakerNumber( int spknum )
    {
        if ( spknum < 5)
        {
            m_mute_level_ttl += m_mute_level_gain;
            m_mute_level_gain = 0;
            m_mute_level_ttl -= m_mute_level_gain;
        }
        else if ( spknum >= 5 && spknum < 10 )
        {
            m_mute_level_ttl += m_mute_level_gain;
            m_mute_level_gain = 4;
            m_mute_level_ttl -= m_mute_level_gain;
        }
        else if ( spknum >= 10 && spknum < 20 )
        {
            m_mute_level_ttl += m_mute_level_gain;
            m_mute_level_gain = 6;
            m_mute_level_ttl -= m_mute_level_gain;
        }
        else
        {
            m_mute_level_ttl += m_mute_level_gain;
            m_mute_level_gain = 7;
            m_mute_level_ttl -= m_mute_level_gain;
        }
    }
}}