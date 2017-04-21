#pragma once
#include <valarray>
#include "fft_wrapper.h"
class MMSE
{
public:
    void Init(int samplerate,int channel);
    void Reset();
    void Process(int16_t* data, size_t size);
private:
    bool ProcessSpec( std::valarray< std::complex<float> >&frec, std::valarray<float>& amplitide, std::valarray<float>& angle );
    FFTWrapper m_fft;
    int m_samplerate = 16000;
    int m_channel = 1;
    std::valarray<float> m_noise;
    std::valarray<float> m_Xk_prev;
    int m_init_num = 0;
    
};