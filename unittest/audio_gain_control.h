#pragma once
#include <cstdint>
#include <complex>
#include <functional>
#include <valarray>
#include <list>
#include "webrtc/common_audio/signal_processing/include/spl_inl.h"
#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#include "webrtc/common_audio/real_fourier.h"
typedef std::function<bool( std::valarray< std::complex<float> >& frec, std::valarray<float>& amplitide, std::valarray<float>& angle ) > SpecAnalyze;

class AudioGainControl
{
    using Complex = std::complex < float >;
public:
    AudioGainControl();
    ~AudioGainControl();
    bool Init( int32_t frame_size, SpecAnalyze analyze, int nOverLapPencent = 50 );
    void ScaleVoice( int16_t* data, int16_t size );
protected:
    int32_t nextpow2( int32_t num );
private:
    float* m_data = nullptr;
    float* m_overLap = nullptr;
    int32_t m_len = 0;
    int32_t m_len1 = 0;  // overlap percent
    int32_t m_len2 = 0;  // frame_size
    float*  m_win = nullptr;
    int32_t m_fftorder = 0;
    std::valarray<Complex> m_spec;
    float*  m_insign = nullptr;
    rtc::scoped_ptr< webrtc::RealFourier > m_fft;
    int32_t m_nFFT = 0;
    std::valarray<float>  m_amplitude;
    std::valarray<float>  m_angle;
    bool    m_bInit = false;
    SpecAnalyze m_SpecAnalyze;
};