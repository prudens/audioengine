#include "audio_normalized.h"
#include <algorithm>
#include <functional>
#include <numeric>
int g_scale = 1000;
AudioNormalized::AudioNormalized()
{


}

AudioNormalized::~AudioNormalized()
{
    //保存到文件里

}

void AudioNormalized::SetLevel( int percent )
{
    m_percent = (1 << 15) * percent / 100;
}

void AudioNormalized::Process( int16_t* data, size_t nsamples )
{
    auto it = std::max_element( data, data + nsamples, [] ( int16_t l, int16_t r ) { return abs( l ) < abs( r ); } );
    if ( it )
    {
        int v = abs( (int)*it );
        m_cur_peak = v;
    }
    float scale = m_scale;
    if ( m_cur_peak * m_scale > 28000 )
    {
        scale = 28000 / m_cur_peak;
        m_scale = ( m_scale * 2 + scale ) / 3;
    }
    for ( size_t i = 0; i < nsamples; i++ )
    {
        data[i] = static_cast<int16_t>( data[i] * scale );
    }
}

void AudioNormalized::Update(bool silent)
{
    if ( !silent )
    {
        m_amp_peaks.push_back( m_cur_peak );
    }
    else
    {
        m_silent_peaks.push_back( m_cur_peak );
    }

    if ( silent && m_silent_peaks.size() > 1000 )
    {
        int mean = std::accumulate( m_silent_peaks.begin(), m_silent_peaks.end(), 0, [] ( int init, int v ) { return init + std::abs( v ); } );
        mean /= m_silent_peaks.size();

        m_max_scale = 400.0f / (float)mean;

        m_silent_peaks = std::vector<int>( m_silent_peaks.begin() + 200, m_silent_peaks.end() );
        printf( "[%.3f]  ", m_scale );
    }


    if ( silent && m_amp_peaks.size() > 1000 )
    {
        auto it = std::max_element( m_amp_peaks.begin(), m_amp_peaks.end() );
        if ( it != m_amp_peaks.end() )
        {
            int base_line = *it;
            m_scale = (float)m_percent / (float)base_line;
            if ( m_scale > std::max(4,static_cast<int>(m_max_scale) ) )
            {
                m_scale = m_max_scale;
            }
        }
        m_amp_peaks = std::vector<int>( m_amp_peaks.begin() + 200, m_amp_peaks.end() );
        printf( "%.3f  ", m_scale );
    }


    
}
