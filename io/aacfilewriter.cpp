#include "aacfilewriter.h"


AACFileWriter::AACFileWriter( const char* filename, int samplerate, int channel )
{
    m_samplerate = samplerate;
    m_channel = channel;
    m_aacfile = fopen( filename, "wb" );
    if (!m_aacfile)
    {
        return;
    }
    CHANNEL_MODE channle_mode = channel == 1 ? MODE_1 : MODE_2;
    AACENC_ERROR ErrorStatus;
    if ( ( ErrorStatus = aacEncOpen( &m_hAacEncoder, 0, 2 ) ) != AACENC_OK )
    {
        return;
    }
    ErrorStatus = aacEncoder_SetParam( m_hAacEncoder, AACENC_AOT, AOT_SBR );
    ErrorStatus = aacEncoder_SetParam( m_hAacEncoder, AACENC_BITRATE, 8*5000 );
    ErrorStatus = aacEncoder_SetParam( m_hAacEncoder, AACENC_SAMPLERATE, samplerate );
    ErrorStatus = aacEncoder_SetParam( m_hAacEncoder, AACENC_CHANNELMODE, channle_mode );
    ErrorStatus = aacEncEncode( m_hAacEncoder, NULL, NULL, NULL, NULL );
    AACENC_InfoStruct encInfo;
    ErrorStatus = aacEncInfo( m_hAacEncoder, &encInfo );
    m_framesize = encInfo.frameLength*channel;
    m_pInputbuf = new char[m_framesize * 2];
    m_outofbyte = new char[encInfo.maxOutBufBytes];

    m_in_eisize = 2;
    m_in_bufsize = m_framesize*sizeof( int16_t );
    m_in_buf_id = IN_AUDIO_DATA;

    m_encinBuf.bufElSizes = &m_in_eisize;
    m_encinBuf.bufferIdentifiers = &m_in_buf_id;
    m_encinBuf.bufSizes = &m_in_bufsize;
    m_encinBuf.numBufs = 1;

    m_out_eisize = 2;
    m_out_bufsize = encInfo.maxOutBufBytes;
    m_out_buf_id = OUT_BITSTREAM_DATA;
    m_encoutBuf.bufElSizes = &m_out_eisize;
    m_encoutBuf.bufferIdentifiers = &m_out_buf_id;
    m_encoutBuf.bufSizes = &m_out_bufsize;
    m_encoutBuf.numBufs = 1;
    m_encoutBuf.bufs = (void **)&m_outofbyte;


    m_in_args.numAncBytes = 0;

    m_bInit = true;
}

AACFileWriter::~AACFileWriter()
{

    if (m_advance_samples>0)
    {
        m_in_args.numInSamples = m_advance_samples * sizeof( int16_t ) / m_channel;
        m_encinBuf.bufs = (void **)&m_pInputbuf;
        m_in_bufsize = m_advance_samples * sizeof( int16_t );
        auto ret = aacEncEncode( m_hAacEncoder, &m_encinBuf, &m_encoutBuf, &m_in_args, &m_out_args );
        if ( ret != AACENC_OK )
        {
            return;
        }
        if ( m_out_args.numOutBytes > 0 )
        {
            fwrite( m_outofbyte, 1, m_out_args.numOutBytes, m_aacfile );
        }
    }


    m_in_bufsize = 0;
    m_encinBuf.bufs = nullptr;
    m_in_args.numInSamples = -1;
    auto ret = aacEncEncode( m_hAacEncoder, &m_encinBuf, &m_encoutBuf, &m_in_args, &m_out_args );
    if ( ret != AACENC_OK )
    {
        return;
    }
    if ( m_out_args.numOutBytes > 0 )
    {
        fwrite( m_outofbyte, 1, m_out_args.numOutBytes, m_aacfile );
    }
    if(m_aacfile) fclose( m_aacfile );
    m_aacfile = nullptr;
    if(m_outofbyte) delete[] m_outofbyte;
    if(m_pInputbuf) delete[] m_pInputbuf;
    aacEncClose( &m_hAacEncoder );
}

void AACFileWriter::Destroy()
{
    delete this;

}

int AACFileWriter::SampleRate() const
{
    return m_samplerate;
}

size_t AACFileWriter::NumChannels() const
{
    return m_channel;
}

size_t AACFileWriter::NumSamples() const
{
    return m_nSamples;
}

void AACFileWriter::WriteSamples( const float* samples, size_t num_samples )
{
    samples = nullptr;
    num_samples = 0;
}

void AACFileWriter::WriteSamples( const int16_t* samples, size_t num_samples )
{
    if ( !m_bInit )
    {
        return;
    }
    m_nSamples += num_samples;
    int tmp = 0;

    if( m_advance_samples + (int)num_samples < m_framesize )
    {
        memcpy( &m_pInputbuf[m_advance_samples * 2], samples, num_samples * 2 );
        m_advance_samples += num_samples;
    }
    else
    {
        tmp = m_framesize - m_advance_samples;
        memcpy( &m_pInputbuf[m_advance_samples*2], samples, tmp * 2 );
        m_in_args.numInSamples = m_framesize * sizeof( int16_t ) / m_channel;
        m_encinBuf.bufs = (void **)&m_pInputbuf;
        m_in_bufsize = m_framesize * sizeof( int16_t );
        auto ret = aacEncEncode( m_hAacEncoder, &m_encinBuf, &m_encoutBuf, &m_in_args, &m_out_args );
        if ( ret != AACENC_OK )
        {
            return;
        }
        if ( m_out_args.numOutBytes > 0 )
        {
            fwrite( m_outofbyte, 1, m_out_args.numOutBytes, m_aacfile );
        }
        memcpy( m_pInputbuf, &samples[tmp], (num_samples - tmp)*2 );
        m_advance_samples = num_samples - tmp;
    }
}
