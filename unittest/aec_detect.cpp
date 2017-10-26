#include "aec_detect.h"
#include <algorithm>
#define  MIN_FRAME_PER_RMS 4
AecDetect::AecDetect()
{
    m_capture_level.reserve( 1000 );
    m_render_level.reserve( 1000 );
}

void AecDetect::ProcessCaptureAudio( int16_t* data, size_t nSample )
{
    m_capture_rms.Process( data, nSample );
    if ( ++m_capture_sample_frame%MIN_FRAME_PER_RMS == 0 )
    {
        int level = m_capture_rms.RMS();
        m_mutex.lock();
        m_capture_level.push_back( level );
        m_mutex.unlock();
        printf( "%d  ", level );
    }
}

void AecDetect::ProcessRenderAudio( int16_t* data, size_t nSample )
{
    m_render_rms.Process( data, nSample );
    if ( ++m_render_sample_frame%MIN_FRAME_PER_RMS == 0 )
    {
        int level = m_render_rms.RMS();
        m_mutex.lock();
        m_render_level.push_back( level );
        m_mutex.unlock();
    }
}

std::vector<std::pair<size_t,size_t>> AecDetect::CalcFeature(std::vector<int> levels)
{
    std::vector<std::pair<size_t, size_t>> idxlist;
    size_t size = levels.size();

    size_t end = (size_t)-1;
    size_t begin = end;
    size_t distance = 2;
    for ( size_t idx = 0; idx < size; idx++ )
    {
        int level = levels[idx];
        if ( level < 40 )
        {
            if ( begin == end )
            {
                begin = idx;
            }
        }
        else
        {
            if ( begin != end && --distance == 0)
            {
                if ( idx - begin >= 5)
                {
                    idxlist.push_back( std::pair<size_t, size_t>( begin, idx ) );
                }
                begin = end;
                distance = 2;
            }
        }
    }
    for ( auto v : idxlist )
    {
        printf( "%d-%d   ", v.first * 40, v.second * 40 );
    }
    return idxlist;
}

int AecDetect::CalcEnergy( int16_t* data, size_t nSample )
{
    float e = 0;
    for ( size_t i = 0; i < nSample; i++ )
    {
        e += data[i] * data[i];
    }
    return static_cast<int>(e/nSample);
}

void AecDetect::SetHeadsetOn( bool on )
{
    m_is_headseton = on;
}

bool AecDetect::IsNeedWebrtcAec()
{
    m_mutex.lock();
    auto capture_list = CalcFeature( m_capture_level );
    auto render_list = CalcFeature( m_render_level );
    m_capture_level.clear();
    m_render_level.clear();
    m_mutex.unlock();
    if ( render_list.size() < 2 || capture_list.size() < 2 )
    {
        return true;
    }
    // 计算播放每句话的间隔
    std::vector<size_t> render_sentence_dist;
    for ( size_t i = 0; i < render_list.size() -1; i++ )
    {
        render_sentence_dist.push_back(render_list[i + 1].first - render_list[i].second);
    }
    
    std::vector<size_t> capture_sentence_dist;
    for ( size_t i = 0; i < capture_list.size() - 1; i++ )
    {
        capture_sentence_dist.push_back( capture_list[i + 1].first - capture_list[i].second );
    }
    size_t size = std::min(capture_sentence_dist.size(),render_sentence_dist.size());
    int result = 0;
    for ( size_t i = 0; i < size; i++ )
    {
        if ( capture_sentence_dist[i] > render_sentence_dist[i])
        {
            result++;
        }
    }

    printf( "result = %d  ", result );
    return result > 2;
}
