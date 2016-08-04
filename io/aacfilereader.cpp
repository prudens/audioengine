#include "aacfilereader.h"
#include <string.h>
AACFileReader::AACFileReader( const char* filename )
{
    m_file = fopen( filename,"rb" );
    if (!m_file)
    {
        return;
    }
    aacDecoderInfo = aacDecoder_Open( TT_MP4_ADTS, 1 );
    if (!aacDecoderInfo)
    {
        return;
    }
    AAC_DECODER_ERROR err;
    UINT bytevalid = 0;
    UINT bufsize =0;
    CStreamInfo* pStreamInfo = nullptr;
    for ( ;; )
    {
        if ( bytevalid == 0 )
        {
            bufsize = fread( m_inBuf, 1, 256, m_file );
            if ( bufsize == 0 )
            {
                break;
            }
            bytevalid = bufsize;
        }
        UINT startpos = bufsize - bytevalid;
        bufsize = bytevalid;
        UCHAR*p = (UCHAR*)m_inBuf + startpos;
        err = aacDecoder_Fill( aacDecoderInfo, &p, &bufsize, &bytevalid );
        if ( err != AAC_DEC_OK )
        {
            printf( "aacDecoder_Fill() failed: 0x%x", err );
            return;
        }
        if ( bytevalid != 0 )
        {
            printf( "bytevalid = %d\n", bytevalid );
        }
        for ( ;; )
        {
            err = aacDecoder_DecodeFrame( aacDecoderInfo, m_outBuf, 4096, 0 );
            if ( err == AAC_DEC_NOT_ENOUGH_BITS )
            {
                break;
            }
            else if ( err != AAC_DEC_OK )
            {
                break;
            }
            if ( !pStreamInfo )
            {
                pStreamInfo = aacDecoder_GetStreamInfo( aacDecoderInfo );
                m_nSamplerate = pStreamInfo->sampleRate;
                m_nChannels = pStreamInfo->numChannels;
            }
            m_nSample += 4096;
        }

    }

    fseek( m_file, 0, SEEK_SET );
    m_bInit = true;
}

AACFileReader::~AACFileReader()
{
    if (!m_bInit)
    {
        return;
    }
    aacDecoder_Close( aacDecoderInfo );
    fclose( m_file );
}

void AACFileReader::Destroy()
{
    delete this;
}

int AACFileReader::SampleRate() const
{
    return m_nSamplerate;
}

size_t AACFileReader::NumChannels() const
{
    return m_nChannels;
}

size_t AACFileReader::NumSamples() const
{
    return m_nSample;
}

size_t AACFileReader::ReadSamples( size_t /*num_samples*/, float* /*samples*/ )
{
    return 0;
}

size_t AACFileReader::ReadSamples( size_t num_samples, int16_t* samples )
{
    if (m_advace_pos + num_samples < 4096 )
    {
        memcpy( samples, m_outBuf, num_samples * 2 );
    }
    int left = m_advace_pos + num_samples - 4096;
    memcpy( samples, m_outBuf + m_advace_pos, left * 2 );
    int need_read = num_samples - left;
    if ( ReadFrame() )
    {
        memcpy( samples + left, m_outBuf, need_read );
    }
    else
    {
        m_readSample += left;
        return left;
    }
    m_readSample += num_samples;
    return num_samples;
}

size_t AACFileReader::RemainSamples() const
{
    return 0;
}

bool AACFileReader::SeekSamples( size_t/* pos*/ )
{
    return false;
}

bool AACFileReader::SeekTime( double/* sec*/ )
{
    return false;
}

bool AACFileReader::SetSpeed( double /*times*/ )
{
    return false;
}

bool AACFileReader::ReadFrame()
{
    AAC_DECODER_ERROR err;

    UINT bufsize = 0;
    for ( ;; )
    {
        if ( bytevalid == 0 )
        {
            bufsize = fread( m_inBuf, 1, 2048, m_file );
            if ( bufsize == 0 )
            {
                return false;
            }
            bytevalid = bufsize;
        }
        UINT startpos = bufsize - bytevalid;
        bufsize = bytevalid;
        UCHAR*p = (UCHAR*)m_inBuf + startpos;
        err = aacDecoder_Fill( aacDecoderInfo, &p, &bufsize, &bytevalid );
        if ( err != AAC_DEC_OK )
        {
            printf( "aacDecoder_Fill() failed: 0x%x", err );
            return false;
        }

        for ( ;; )
        {
            err = aacDecoder_DecodeFrame( aacDecoderInfo, m_outBuf, 4096, 0 );
            if ( err == AAC_DEC_NOT_ENOUGH_BITS )
            {
                break;
            }
            else if ( err != AAC_DEC_OK )
            {
                return false;
            }
        }

    }
}




