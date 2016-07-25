#pragma once
/*!
 * \file audio_low_pass_filter.h
 * \date 2016/06/01 13:31
 *
 * \author zhangnaigan
 * Contact: zhangng@snailgame.net
 *
 * \brief 低通滤波器，一般不使用，如果语音效果很不好，可以开启，降低音质的同时可以消除很大部分的噪音
 *
 * TODO: 目前是通过matlab生成参数，后面可根据需要通过cpp里的注释代码修改。
 *
 * \note
*/
#include "stdint.h"
#include "webrtc/common_audio/fir_filter.h"

#define PROCESSING_LENGTH 480
class LowPassFilter
{
public:
    const static int kFFTSize = 160;
    const static int kWindowSize = 256;
    const static int kFilterHz = 64;
    LowPassFilter();
    ~LowPassFilter();
public:
    void Enable( bool bEnable );
    bool IsEnable()const;
    void SetSampleRate(int samplerate);
    void Processing( const int16_t* in, int16_t* out, size_t len );
private:
    webrtc::FIRFilter* m_filter;
    bool m_bEnable;
    float* m_fin;
    float* m_fout;
    float* m_pFilterCocc;

};