#pragma once
#include <cstdint>
#include <vector>
#include <valarray>
#include <utility>
#include "webrtc/common_audio/fir_filter.h"


class FindPeaks
{
public:
    using size_type = size_t;
    using value_type = float;
    using result_type = std::vector < std::pair<value_type, size_type> >;

    FindPeaks();
    ~FindPeaks();
    result_type Process( value_type* freq_frame, size_type size );
    void SetMinPeakDistance(int distance);
    void SetMinPeakHeight(value_type height);
    void SetPeakThreshold( value_type th );
public:
    void Filter( const float* const freq_frame, size_type size );//Æ½»¬Ò»Ð©¡£
    void FindAllPeaks();
    void RemovePeaksBelowThreshold( value_type minT, value_type minH, size_type minW );

    std::valarray<value_type> m_freq_frame_filter;
    std::vector<size_type> m_ipks;
    int  m_min_peak_distance = 5;
    value_type m_peak_threshold = 0.0f;
    value_type m_min_peak_height = 0.05f;
};