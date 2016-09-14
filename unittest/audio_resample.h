#pragma once
#include <cstdint>
// #include "webrtc/common_audio/signal_processing/include/spl_inl.h"
// #include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#include "webrtc/common_audio/resampler/include/resampler.h"
class AudioResample
{
public:
    AudioResample();
    bool Reset(int32_t inSampleRate, int16_t inChannel, int32_t outSamplerate, int16_t outChannel);
    bool Process(int16_t* inBuf,size_t inSamples, int16_t* outBuf, size_t outSamples);

private:
    webrtc::Resampler m_ResampleImpl;
    int32_t m_inSamplerate = 0;
    int32_t m_outSamplerate = 0;
    int16_t m_inChannel = 0;
    int16_t m_outChannel = 0;
};