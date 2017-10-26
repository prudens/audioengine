#include "findpeaks.h"
#include <cassert>
#include <algorithm>
#include <functional>
FindPeaks::FindPeaks()
{

}

FindPeaks::~FindPeaks()
{

}

std::vector<std::pair<FindPeaks::value_type, FindPeaks::size_type> > FindPeaks::Process( value_type* freq_frame, size_type size )
{
    std::vector<std::pair<FindPeaks::value_type, FindPeaks::size_type> > res;
    assert( freq_frame && size > 1 );
    
    Filter( freq_frame, size );
    FindAllPeaks();
    RemovePeaksBelowThreshold(m_peak_threshold, m_min_peak_height, m_min_peak_distance);
    for ( size_type i = 0; i < m_ipks.size(); i++ )
    {
        res.push_back( { m_freq_frame_filter[m_ipks[i]], m_ipks[i] } );
    }

    return res;
}

void FindPeaks::Filter( const float* const freq_frame, size_type size )
{
    if ( size != m_freq_frame_filter.size() )
    {
        m_freq_frame_filter.resize( size );
    }
    for ( size_type i = 1; i < size; i++ )
    {
        m_freq_frame_filter[i] = freq_frame[i];
    }
#if 1
    return;
#else
    // 根据需要来决定是否使用下面的平滑滤波器
    float b[] = {0.15f, 0.2f, 0.3f, 0.2f, 0.15f};
    const static size_type corr_len = sizeof( b )/sizeof(b[0]);
    for ( size_type i = 0; i < corr_len-1; i++ )
    {
        m_freq_frame_filter[i] += freq_frame[i - 0] * b[corr_len - 1];
        if ( i - 0 == 0 ) continue;
        m_freq_frame_filter[i] += freq_frame[i - 1] * b[corr_len - 2];
        if ( i - 1 == 0 ) continue;
        m_freq_frame_filter[i] += freq_frame[i - 2] * b[corr_len - 3];
        if ( i - 2 == 0 ) continue;
        m_freq_frame_filter[i] += freq_frame[i - 3] * b[corr_len - 4];
    }

    for ( size_type i = corr_len-1; i < size; i++ )
    {
        m_freq_frame_filter[i] += freq_frame[i - 0] * b[corr_len - 1];
        m_freq_frame_filter[i] += freq_frame[i - 1] * b[corr_len - 2];
        m_freq_frame_filter[i] += freq_frame[i - 2] * b[corr_len - 3];
        m_freq_frame_filter[i] += freq_frame[i - 3] * b[corr_len - 4];
        m_freq_frame_filter[i] += freq_frame[i - 4] * b[corr_len - 5];
    }
#endif
}

void FindPeaks::FindAllPeaks()
{
    size_type size = m_freq_frame_filter.size();
    m_ipks.clear();
    for ( size_type i = 1; i < size-1;i++ )
    {
        if ( m_freq_frame_filter[i] >= m_freq_frame_filter[i-1] &&
             m_freq_frame_filter[i] >= m_freq_frame_filter[i+1] )
        {
            m_ipks.push_back(i);
        }
    }
}

void FindPeaks::RemovePeaksBelowThreshold( value_type minT, value_type minH, size_type minW )
{
    for ( auto it = m_ipks.begin(); it != m_ipks.end(); )
    {
        if ( m_freq_frame_filter[*it] < minH )
        {
            it = m_ipks.erase( it );
            continue;
        }
        else
        {
            it++;
        }
    }
    size_type size = m_ipks.size();
    std::vector<size_type> res;
    for ( size_type i = 0; i < size; i++ )
    {
        auto idx = m_ipks[i];
        auto v = std::max( m_freq_frame_filter[idx - 1],
                           m_freq_frame_filter[idx + 1] );
        if ( m_freq_frame_filter[idx] - v > minT )
        {
            res.push_back( m_ipks[i] );
        }
    }

    std::vector<std::pair<value_type, size_type>> pks;
    pks.resize( res.size() );
    for ( size_type i = 0; i < res.size(); i++ )
    {
        pks[i].first = m_freq_frame_filter[res[i]];
        pks[i].second = res[i];
    }
    //降序排序
    std::sort( pks.begin(), pks.end(), [] ( std::pair<value_type, size_type>& l, std::pair<value_type, size_type>& r)
    {
        return r.first < l.first;
    });
    size = pks.size();
    std::vector<int> idelete( size, 0 );
    for ( size_type i = 0; i < size; i++ )
    {
        if (idelete[i] == 0)
        {
            for ( size_type j = 0; j < idelete.size(); j++ )
            {
                idelete[j] = idelete[j] || ( pks[j].second + minW >= pks[i].second ) && ( pks[j].second <= pks[i].second + minW );
            }
            idelete[i] = 0;
        }
    }

    m_ipks.clear();
    size = idelete.size();
    for ( size_type i = 0; i < size; i++ )
    {
        if (!idelete[i])
        {
            m_ipks.push_back( pks[i].second );
        }
    }

    std::sort( m_ipks.begin(), m_ipks.end() );//升序排序

}

void FindPeaks::SetMinPeakDistance( int distance )
{
    m_min_peak_distance = distance;
}

void FindPeaks::SetMinPeakHeight( value_type height )
{
    m_min_peak_height = height;
}

void FindPeaks::SetPeakThreshold( value_type th )
{
    m_peak_threshold = th;
}


