#include "pcm_file.h"
PCMFileReader::PCMFileReader( const char* filename)
{
    m_read_file = fopen( filename, "rb+" );
    if (!m_read_file)
    {
        return;
    }
    m_num_samples = fseek( m_read_file, 0, SEEK_END )/2;
    fseek( m_read_file, 0, SEEK_SET );
    m_remain_samples = m_num_samples;
    m_init = true;
}

void PCMFileReader::Destroy()
{
    if (m_read_file)
    {
        fclose( m_read_file );
    }
    delete this;
}

size_t PCMFileReader::ReadSamples( size_t num_samples, float* samples )
{
    if (!m_init)
    {
        return 0;
    }
    int read_len = fread( samples, sizeof( float ), num_samples, m_read_file );
    m_remain_samples -= read_len;
    return read_len;
}

size_t PCMFileReader::ReadSamples( size_t num_samples, int16_t* samples )
{
    if ( !m_init )
    {
        return 0;
    }
    int read_len = fread( samples, sizeof( int16_t ), num_samples, m_read_file );
    m_remain_samples -= read_len;
    return read_len;
}

bool PCMFileReader::SeekSamples( size_t pos )
{
    fseek( m_read_file, pos * 2, SEEK_SET );
    return true;
}

PCMFileWriter::PCMFileWriter( const char* filename, int samplerate, int channel )
{
    m_sample_rate = samplerate;
    m_channel = channel;
    m_writer_file = fopen( filename, "wb+" );
    if( !m_writer_file)
    {
        return;
    }
    m_init;
}

void PCMFileWriter::Destroy()
{
    if (m_writer_file)
    {
        fclose( m_writer_file );
    }

    delete this;
}

void PCMFileWriter::WriteSamples( const float* samples, size_t num_samples )
{
    if (!m_init)
    {
        return;
    }
    int num = fwrite( samples, sizeof( float ), num_samples, m_writer_file );
    m_num_samples += num;
}

void PCMFileWriter::WriteSamples( const int16_t* samples, size_t num_samples )
{
    if ( !m_init )
    {
        return;
    }
    int num = fwrite( samples, sizeof( int16_t ), num_samples, m_writer_file );
    m_num_samples += num;
}

