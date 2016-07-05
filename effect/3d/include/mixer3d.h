#pragma once
#include <complex>
#include <cstdint>
#include <mutex>
#include <list>
#include "base/audio_util.h"
#include "base/circular_buffer.hpp"

class AudioSampleBuffer
{
public:
    AudioSampleBuffer( int16_t*pData, size_t nSamples )
    {
        m_nSample = nSamples;
        m_pData[0] = new float[nSamples];
        m_pData[1] = new float[nSamples];
        if ( pData )
        {
            for ( size_t i = 0; i < nSamples; i++ )
            {
                m_pData[0][i] = S16ToFloat( pData[i * 2] );
                m_pData[1][i] = S16ToFloat( pData[i * 2 + 1] );
            }
        }
        else
        {
            memset( m_pData[0], 0, nSamples*sizeof( float ) );
            memset( m_pData[1], 0, nSamples*sizeof( float ) );
        }
    }
    ~AudioSampleBuffer()
    {
        delete[] m_pData[0];
        delete[] m_pData[1];

    }
    const float* getReadPointer( int index )
    {
        return m_pData[index];
    }
    float* getWritePointer( int index )
    {
        return m_pData[index];
    }

    int getNumSamples()
    {
        return m_nSample;
    }
private:
    float* m_pData[2];
    size_t m_nSample;
};


class Mixer3D
{
    using value_type = float;
    using Complex = std::complex<value_type>;
    using lockguard = std::lock_guard<std::mutex>;
    static const int nFFT = 1024;
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
    void convolution( const float*input, Complex*irc, float *output, int nFFT, int nSig );
private:

    AudioSampleBuffer m_prevBuf;
    CircularAudioBuffer m_audio_buffer_in;
    CircularAudioBuffer m_audio_buffer_out;

    Complex m_fltl[nFFT];
    Complex m_fltr[nFFT];
    std::mutex   m_lock;
    size_t m_nSamplerate;
};