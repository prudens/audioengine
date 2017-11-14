#pragma once
#include <cstdint>
#include "webrtc/common_audio/resampler/include/resampler.h"
#include "webrtc/common_audio/resampler/include/push_resampler.h"
class AudioResample
{
public:
    AudioResample();
	~AudioResample();
    static bool ToMono( int16_t*src, int num_samples,int16_t*dst);
    static bool ToMono( int16_t*src, int num_samples);
    static bool ToMono( int16_t*src, int16_t num_samples, int16_t*left, int16_t*right );
    static bool Tostereo( int16_t*src, int num_samples, int16_t*dst );
    static bool Tostereo( int16_t*src, int num_samples );
    static bool Tostereo( int16_t* left, int16_t*right, int num_samples, int16_t*dst );
    bool Reset(int32_t inSampleRate, int16_t inChannel, int32_t outSamplerate, int16_t outChannel);
    int  Process(int16_t* inBuf,size_t inSamples, int16_t* outBuf, size_t outSamples);
	int  OutSamplerate(){ return m_outSamplerate; }
	int  InSamplerate(){ return m_inSamplerate; }
	int  OutChannel(){ return m_outChannel; }
	int  InChannel(){ return m_inChannel; }
private:
    webrtc::Resampler* m_resampler = nullptr;
	webrtc::PushResampler<int16_t>* m_push_resample = nullptr;
    int32_t m_inSamplerate = 0;
    int32_t m_outSamplerate = 0;
    int16_t m_inChannel = 0;
    int16_t m_outChannel = 0;
};