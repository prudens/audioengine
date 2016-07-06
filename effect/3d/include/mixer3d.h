#pragma once
/*
 * @file mixer3d.h
 * @date 2016/07/06 13:14
 *
 * @author zhangnaigan
 * @contact zhangng@snailgame.net
 * @brief 对mit-hrtf库的简单封装
 * @note
 */
#include <complex>
#include <cstdint>
#include <mutex>
#include <list>
#include "base/audio_util.h"
#include "base/circular_buffer.hpp"




class Mixer3D
{
    using value_type = float;
    using Complex = std::complex<value_type>;
    using lockguard = std::lock_guard<std::mutex>;
    static const int nFFT = 1024;
    class AudioSampleBuffer
    {
    public:
        AudioSampleBuffer( int16_t*pData, size_t nSamples );
        ~AudioSampleBuffer();
        const float* getReadPointer( int index );
        float* getWritePointer( int index )
        {
            return m_pData[index];
        }

        int getNumSamples();
    private:
        float* m_pData[2];
        size_t m_nSample;
    };
public:
    Mixer3D();
    Mixer3D( size_t samplerate );
    ~Mixer3D();
public:
    bool SetFormat( size_t samplerate );
    bool updateAngles( int nAzimuth, int nElevation );
    void AddProcessData( int16_t* pData, size_t samples);
    size_t GetProcessData(int16_t*pData, size_t samples);
private:
    void ProcessBlock( AudioSampleBuffer &buffer);
    /**
     * @brief     卷积函数，fft性能较差，有待改进，nfft必须是2的N阶
     * @return    void
     * @param     const float * input
     * @param     Complex * irc
     * @param     float * output
     * @param     int nFFT
     * @param     int nSig
     */
    void convolution( const float*input, Complex*irc, float *output, int nFFT, int nSig );
private:

    AudioSampleBuffer                    m_prevBuffer;
    CircularAudioBuffer                  m_audio_buffer_in;
    CircularAudioBuffer                  m_audio_buffer_out;

    Complex                              m_cFilterL[nFFT];
    Complex                              m_cFilterR[nFFT];
    Complex                              m_cInput[nFFT];
    Complex                              m_cOutput[nFFT];
    float                                m_foutL[nFFT];
    float                                m_foutR[nFFT];

    std::mutex                           m_lock;
    size_t                               m_nSamplerate;
};