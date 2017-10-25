#include "mmse.h"
#include <boost/math/special_functions/bessel.hpp>
void MMSE::Init( int samplerate, int channel )
{
    m_fft.Init( samplerate / 100, std::bind( &MMSE::ProcessSpec, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
    m_noise.resize(m_fft.FFTSize());
    m_noise = 0.01f;
    m_Xk_prev = m_noise;
    m_samplerate = samplerate;
    m_channel = channel;
    m_init_num = 0;
}

void MMSE::Process( int16_t* data, size_t size )
{
    m_fft.Process( data, size );
}

bool MMSE::ProcessSpec( std::valarray< std::complex<float> >&, std::valarray<float>& sig, std::valarray<float>& )
{
//    return true;
    m_init_num++;
    if ( m_init_num < 6 )
    {
        m_noise += sig;
        return false;
    }
    if (m_init_num == 6)
    {
        m_noise += sig;
        m_noise /= 6;
        m_noise *= m_noise;
    }

    static float c = 0.8862f; //sqrt(PI)/2
    static float aa = 0.98f;
    static float ksi_min = 10 ^ ( -25 / 10 );// note that in Chap. 7, ref.[17], ksi_min( dB ) = -15 dB is recommended
    auto amplitide = pow( sig, 2.0f );
    std::valarray<float> gammak = amplitide / m_noise;//posteriori SNR
    gammak = gammak.apply( [] ( float v ) { return std::min( v, 40.0f ); } );

    auto ksi = m_Xk_prev / m_noise * aa + ( 1 - aa ) * gammak.apply( [] ( float v ) { return std::max( v - 1.0f, 0.0f ); } );
    ksi = ksi.apply( [] ( float v ) { return std::max( v, ksi_min ); } );
    auto log_sigma_k = gammak*ksi / ( ksi + 1.0f ) - log(  ksi + 1.0f );
    auto vad_decision = log_sigma_k.sum() / sig.size();
    if ( vad_decision < 0.2f )
    {
        m_noise = m_noise * 0.98f + amplitide * ( 0.02f );
    }

    auto vk = ksi*gammak / ( ksi + 1.0f );
    auto j0 = vk.apply( [] ( float v ) { return (float)boost::math::cyl_bessel_i( 0.0f, v / 2 ); } );
    auto j1 = vk.apply( [] ( float v ) { return  (float)boost::math::cyl_bessel_i( 1.0f, v / 2 ); } );
    auto C = exp( vk * -0.5f );
    
    auto A = sqrt(vk)  * c * C / gammak;
    auto B = ( vk + 1.f )*j0 + vk*j1;
    auto hw = A*B;
    sig *= hw; //×îÖÕ½á¹û¡£
    m_Xk_prev = sig * sig;
    return true;
}

void MMSE::Reset()
{
    m_noise = 0;
    m_init_num = 0;
    m_Xk_prev = 0;
}


