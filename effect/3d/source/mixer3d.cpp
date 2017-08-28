#include "effect/3d/include/mixer3d.h"
#include "effect/3d/include/mit_hrtf_lib.h"
#include "base/fft.h"
#include <assert.h>
#include <algorithm>
Mixer3D::Mixer3D() :m_prevBuffer(nullptr, nFFT / 2)
,m_audio_buffer_in(nFFT * 2)
,m_audio_buffer_out(nFFT*2)
{

}

Mixer3D::Mixer3D( size_t samplerate ) :m_prevBuffer( nullptr, nFFT / 2 )
, m_audio_buffer_in( nFFT * 2 )
, m_audio_buffer_out( nFFT * 2 )
, m_nSamplerate( samplerate )
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
            m_cFilterL[i] = Complex( pLeft[i] );
            m_cFilterR[i] = Complex( pRight[i] );
        }
        else
        {
            m_cFilterL[i] = Complex( 0 );
            m_cFilterR[i] = Complex( 0 );
        }
    }
    CFFT::Forward( m_cFilterL, nFFT );
    CFFT::Forward( m_cFilterR, nFFT );
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

    const float *proRdL = m_prevBuffer.getReadPointer( 0 );
    const float * proRdR = m_prevBuffer.getReadPointer( 1 );
    float *proWtL = m_prevBuffer.getWritePointer( 0 );
    float *proWtR = m_prevBuffer.getWritePointer( 1 );

    int nSig = bufSize;

    convolution( inL, m_cFilterL, m_foutL, nFFT, nSig );
    convolution( inR, m_cFilterR, m_foutR, nFFT, nSig );
    for ( int i = 0; i < bufSize; i++ )
    {
        outL[i] = ( m_foutL[i] + proRdL[i] ) / 2;
        outR[i] = ( m_foutR[i] + proRdR[i] ) / 2;
        proWtL[i] = m_foutL[i + bufSize];
        proWtR[i] = m_foutR[i + bufSize];
    }
}

void Mixer3D::convolution( const float*input, Complex*irc, float *output, int len, int nSig )
{
    for ( int i = 0; i < len; i++ )
    {
        if ( i < nSig )
        {
            m_cInput[i] = Complex( (float)input[i] );
        }
        else
        {
            m_cInput[i] = Complex( 0 );
        }
    }

    CFFT::Forward( m_cInput, len);
    for ( int i = 0; i < len; i++ )
    {
        m_cOutput[i] = m_cInput[i] * irc[i];
    }

    CFFT::Inverse( m_cOutput, len);

    for ( int i = 0; i < len; i++ )
    {
        output[i] = (float)( m_cOutput[i].real() );
    }
}



Mixer3D::AudioSampleBuffer::AudioSampleBuffer( int16_t*pData, size_t nSamples )
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

Mixer3D::AudioSampleBuffer::~AudioSampleBuffer()
{
    delete[] m_pData[0];
    delete[] m_pData[1];
}

const float* Mixer3D::AudioSampleBuffer::getReadPointer( int index )
{
    return m_pData[index];
}

int Mixer3D::AudioSampleBuffer::getNumSamples()
{
    return m_nSample;
}
