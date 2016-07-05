#include "effect/3d/include/mixer3d.h"
#include "effect/3d/include/mit_hrtf_lib.h"
#include "base/fft.h"
#include <assert.h>
#include <algorithm>
Mixer3D::Mixer3D() :m_prevBuf(nullptr, nFFT / 2)
,m_audio_buffer_in(nFFT * 2)
,m_audio_buffer_out(nFFT*2)
{

}

Mixer3D::Mixer3D( size_t samplerate ) : m_nSamplerate( samplerate )
, m_prevBuf( nullptr, nFFT / 2 )
, m_audio_buffer_in( nFFT * 2 )
, m_audio_buffer_out( nFFT * 2 )
{

}

Mixer3D::~Mixer3D()
{

}

bool Mixer3D::SetFormat( size_t samplerate )
{
    m_nSamplerate = samplerate;
    int nTaps = mit_hrtf_availability(0,0,samplerate,0);
    return nTaps < 0;
}

bool Mixer3D::updateAngles( int nAzimuth, int nElevation )
{
    lockguard lg( m_lock );
    float pLeft[nFFT] = { 0 };
    float pRight[nFFT] = { 0 };
    int nFil = mit_hrtf_get( &nAzimuth, &nElevation, 48000, 0, pLeft, pRight );
    if ( nFil == 0 ) return false;
    for ( int i = 0; i < nFFT; i++ )
    {
        if ( i < nFil )
        {
            m_fltl[i] = Complex( pLeft[i] );
            m_fltr[i] = Complex( pRight[i] );
        }
        else
        {
            m_fltl[i] = Complex( 0 );
            m_fltr[i] = Complex( 0 );
        }
    }
    CFFT::Forward( m_fltl, nFFT );
    CFFT::Forward( m_fltr, nFFT );
    return true;
}

void Mixer3D::AddProcessData(  int16_t* pData, size_t samples )
{
    lockguard lg( m_lock );
    if (m_audio_buffer_in.writeSizeRemaining()>= samples)
    {
        m_audio_buffer_in.write( pData, samples );
    }
    else
    {
        printf( "error\n" );
    }

    if ( m_audio_buffer_in.readSizeRemaining() < nFFT )
    {
        return;
    }
    int16_t buf[nFFT];
    size_t readlen = m_audio_buffer_in.read( buf, nFFT );
    assert( readlen == nFFT );
    AudioSampleBuffer buffer( buf, nFFT / 2 );
    ProcessBlock( buffer );
    const float*pLeft = buffer.getReadPointer( 0 );
    const float*pRight = buffer.getReadPointer( 1 );

    for ( size_t i = 0; i < readlen / 2; i++ )
    {
        buf[i * 2] = FloatToS16( pLeft[i] );
        buf[i * 2 + 1] = FloatToS16( pRight[i] );
    }
    assert( m_audio_buffer_out.writeSizeRemaining() >= nFFT );
    m_audio_buffer_out.write( buf, nFFT );
}

size_t Mixer3D::GetProcessData( int16_t*pData, size_t samples )
{
    lockguard lg( m_lock );
    if ( samples <= m_audio_buffer_out.readSizeRemaining() )
    {
        assert( samples == m_audio_buffer_out.read( pData, samples ) );
        return samples;
    }
    return 0;
}


void Mixer3D::ProcessBlock( AudioSampleBuffer& buffer )
{
    const float*inL = buffer.getReadPointer( 0 );
    const float*inR = buffer.getReadPointer( 1 );
    float *outL = buffer.getWritePointer( 0 );
    float *outR = buffer.getWritePointer( 1 );

    int bufSize = buffer.getNumSamples();

    const float *proRdL = m_prevBuf.getReadPointer( 0 );
    const float * proRdR = m_prevBuf.getReadPointer( 1 );
    float *proWtL = m_prevBuf.getWritePointer( 0 );
    float *proWtR = m_prevBuf.getWritePointer( 1 );

    int nSig = bufSize;
    float *outLp = new float[nFFT];
    float *outRp = new float[nFFT];
    convolution( inL, m_fltl, outLp, nFFT, nSig );
    convolution( inR, m_fltr, outRp, nFFT, nSig );
    for ( int i = 0; i < bufSize; i++ )
    {
        outL[i] = ( outLp[i] + proRdL[i] ) / 2;
        outR[i] = ( outRp[i] + proRdR[i] ) / 2;
        proWtL[i] = outLp[i + bufSize];
        proWtR[i] = outRp[i + bufSize];
    }
    delete[] outLp;
    delete[] outRp;
}

void Mixer3D::convolution( const float*input, Complex*irc, float *output, int nFFT, int nSig )
{

    Complex*inc, *outc;
    inc = new Complex[nFFT];
    for ( int i = 0; i < nFFT; i++ )
    {
        if ( i < nSig )
        {
            inc[i] = Complex( (float)input[i] );
        }
        else
        {
            inc[i] = Complex( 0 );
        }
    }

    CFFT::Forward( inc, nFFT );
    outc = new Complex[nFFT];
    for ( int i = 0; i < nFFT; i++ )
    {
        outc[i] = inc[i] * irc[i];
    }

    CFFT::Inverse( outc, nFFT );

    for ( int i = 0; i < nFFT; i++ )
    {
        output[i] = (float)( outc[i].real() );
    }
    delete[] inc;
    delete[] outc;
    
}


