#include <cstring>
#include "mp3filewriter.h"

Mp3FileWriter::Mp3FileWriter(const char* filename, int samplerate, int channel )
    :m_OutBuffer( 8 * 1024 )
{
    m_samplerate = samplerate;
    m_channel = channel;
    
    if (filename)
    {
       m_mp3file = fopen( filename, "wb" );
    }
    if (!m_mp3file)
    {
        return;
    }
    m_lame = lame_init();
    
    if (!m_lame)
    {
        return;
    }

    int ret = lame_set_in_samplerate( m_lame, m_samplerate );
    if (ret != 0)
    {
        return;
    }
    ret = lame_set_num_channels( m_lame, m_channel );
    if (ret != 0)
    {
        return;
    }
    ret = lame_set_VBR_mean_bitrate_kbps( m_lame,128 );
    ret = lame_init_params( m_lame );
    if ( ret != 0 )
    {
        return;
    }

    m_bInit = true;
}

Mp3FileWriter::~Mp3FileWriter()
{
    if ( !m_bInit )
    {
        return;
    }
    if ( m_lame )
    {
        if ( m_OutBuffer.unused() < 7200 )
        {
            fwrite( m_OutBuffer.begin(), 1, m_OutBuffer.used(), m_mp3file );
            m_OutBuffer.advance( -m_OutBuffer.used() );
        }
        int len = lame_encode_flush( m_lame, m_OutBuffer.current(), m_OutBuffer.unused() );
        m_OutBuffer.advance( len );
        fwrite( m_OutBuffer.begin(), m_OutBuffer.used(), 1, m_mp3file );
        m_OutBuffer.advance( -m_OutBuffer.used() );
        len = lame_get_lametag_frame( m_lame, 0, 0 );
        len = lame_get_lametag_frame( m_lame, m_OutBuffer.begin(), m_OutBuffer.unused() );
        fseek( m_mp3file, 0, SEEK_SET );
        fwrite( m_OutBuffer.begin(), 1, len, m_mp3file );
        fclose( m_mp3file );
        lame_close( m_lame );
    }
}

void Mp3FileWriter::Destroy()
{
    delete this;
}

int Mp3FileWriter::SampleRate() const
{
    return m_samplerate;
}

size_t Mp3FileWriter::NumChannels() const
{
    return m_channel;
}

size_t Mp3FileWriter::NumSamples() const
{
    return m_nSamples;
}

void Mp3FileWriter::WriteSamples( const float* samples, size_t num_samples )
{
    if ( !m_bInit )
    {
        return;
    }
    int len;
    if (m_channel == 1)
    {
        len = lame_encode_buffer_ieee_float( m_lame, samples, nullptr, num_samples, m_OutBuffer.current(), m_OutBuffer.unused() );
    }
    else
    {
        len = lame_encode_buffer_interleaved_ieee_float( m_lame, (float*)samples, num_samples / m_channel, m_OutBuffer.current(), m_OutBuffer.unused() );
    }
    if ( len <= 0 )
    {
        return;
    }
    m_OutBuffer.advance( len );
    if ( m_OutBuffer.used() > 7 * 1024 )
    {
        fwrite( m_OutBuffer.begin(), 1, m_OutBuffer.used(), m_mp3file );
        m_OutBuffer.advance( -m_OutBuffer.used() );
    }
    m_nSamples += num_samples;
}

void Mp3FileWriter::WriteSamples( const int16_t* samples, size_t num_samples )
{
    if ( !m_bInit )
    {
        return;
    }
    int len;
    m_nSamples += num_samples;
    if (m_channel==1)
    {
        len = lame_encode_buffer( m_lame, samples, nullptr, num_samples, m_OutBuffer.current(), m_OutBuffer.unused() );
    }
    else
    {
        len = lame_encode_buffer_interleaved( m_lame, (int16_t*)samples, num_samples / m_channel, m_OutBuffer.current(), m_OutBuffer.unused() );
    }
    if ( len <= 0 )
    {
        return;
    }
    m_OutBuffer.advance( len );
    if ( m_OutBuffer.used() > 7 * 1024 )
    {
        fwrite( m_OutBuffer.begin(), 1, m_OutBuffer.used(), m_mp3file );
        m_OutBuffer.advance( -m_OutBuffer.used() );
    }

}

