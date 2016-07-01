//   fft.h - declaration of class
//   of fast Fourier transform - FFT
//
//   The code is property of LIBROW
//   You can use it on your own
//   When utilizing credit LIBROW site

#ifndef _FFT_H_
#define _FFT_H_

//   Include complex numbers header
 #include <complex>

class CFFT
{
public:
	//   FORWARD FOURIER TRANSFORM
	//     Input  - input data
	//     Output - transform result
	//     N      - length of both input data and result

	static bool Forward(const std::complex<float> *const Input, std::complex<float> *const Output, const unsigned int N);
    static bool Forward( const std::complex<double> *const Input, std::complex<double> *const Output, const unsigned int N );
    static bool Forward( const std::complex<long double> *const Input, std::complex<long double> *const Output, const unsigned int N );

	//   FORWARD FOURIER TRANSFORM, INPLACE VERSION
	//     Data - both input data and output
	//     N    - length of input data

    static bool Forward( std::complex<float> *const Data, const unsigned int N );
    static bool Forward( std::complex<double> *const Data, const unsigned int N );
    static bool Forward( std::complex<long double> *const Data, const unsigned int N );

	//   INVERSE FOURIER TRANSFORM
	//     Input  - input data
	//     Output - transform result
	//     N      - length of both input data and result
	//     Scale  - if to scale result

    static bool Inverse( const  std::complex<float> *const Input, std::complex<float> *const Output, const unsigned int N, const bool Scale = true );
    static bool Inverse( const  std::complex<double> *const Input, std::complex<double> *const Output, const unsigned int N, const bool Scale = true );
    static bool Inverse( const  std::complex<long double> *const Input, std::complex<long double> *const Output, const unsigned int N, const bool Scale = true );

	//   INVERSE FOURIER TRANSFORM, INPLACE VERSION
	//     Data  - both input data and output
	//     N     - length of both input data and result
	//     Scale - if to scale result

    static bool Inverse( std::complex<float> *const Data, const unsigned int N, const bool Scale = true );
    static bool Inverse( std::complex<double> *const Data, const unsigned int N, const bool Scale = true );
    static bool Inverse( std::complex<long double> *const Data, const unsigned int N, const bool Scale = true );
};

#endif
