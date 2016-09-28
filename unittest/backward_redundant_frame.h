#pragma once
/*!
 * \file backward_redundant_frame.h
 *
 * \author zhangnaigan
 * \date 九月 2016
 *
 * 后向携带冗余信息帧，即当前待播放的帧携带了前面N次帧,处理之后重采样成24000Hz,两个频域叠加
 */
#include <cstdint>
//#include "macrodef.h"
#include "webrtc/common_audio/resampler/include/resampler.h"
class BackwardRedunantFrame
{
    static const int NUM_FRAME = 4;             // 四帧合一
    static const int OUT_SAMPLE_RATE = 48000;   // 最终输出的采样率，给AAC编码
    static const int BASE_SAMPLE_RATE = 24000;  // 降采样到24kHz，这样两个在频域叠加之后，恰好有24kHz,AAC需要保证至少能达到16kHz的频率信息，否则信息会被截断

    struct BRFrame 
    {
        int       noise_type;                      // 语音类型
        int16_t   data[BASE_SAMPLE_RATE/100];      //240,双声道
    };

public:
    BackwardRedunantFrame();
    ~BackwardRedunantFrame();
    bool Init(int inSamplerate);
    void Process( int16_t* data, int nSamples, int16_t*outData, int &outSample );
private:
    BRFrame* brFrames[NUM_FRAME];
    webrtc::Resampler resample24000_;
};