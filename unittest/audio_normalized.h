#pragma once
/*!
 * \file audio_normalized.h
 * \brief 语音数据归一化处理。简单的乘以一个比例，与agc无关
 *
 * \author zhangnaigan
 * \date 2017/08/01
 * \version audio_normalized.h
 * 
 */
#include <cstdint>
#include <vector>
class AudioNormalized
{
public:
    AudioNormalized();
    ~AudioNormalized();
public:
    void SetLevel(int percent );//0-100
    void Process(int16_t* data, size_t nsamples);
    void Update(bool silent);
private:
    int m_percent = 1<<14;
    std::vector < int > m_amp_peaks;
    std::vector<int> m_silent_peaks;
    float m_scale = 1.0f;
    int m_cur_peak = 0;
    float m_max_scale = 1.0f;
};