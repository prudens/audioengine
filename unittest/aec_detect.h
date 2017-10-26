#pragma once
#include <cstdint>
#include <vector>
#include <mutex>
#include "webrtc/modules/audio_processing/rms_level.h"
class AecDetect
{
public:
    AecDetect();
    void SetHeadsetOn( bool on );
    void ProcessCaptureAudio( int16_t* data, size_t nSample );
    void ProcessRenderAudio( int16_t* data, size_t nSample );
    bool IsNeedWebrtcAec();
    static int CalcEnergy( int16_t* data, size_t nSample );
    static std::vector<std::pair<size_t, size_t>> CalcFeature( std::vector<int> levels );
private:
    std::vector<int> m_render_level;
    std::vector<int> m_capture_level;
    webrtc::RMSLevel m_render_rms;
    webrtc::RMSLevel m_capture_rms;
    int m_capture_sample_frame = 0;
    int m_render_sample_frame = 0;
    bool m_is_headseton = false;
    std::mutex m_mutex;
};