#include "mp3filereader.h"
#include <stdio.h>
#include <algorithm>

#define CHECK_FAIL(ret)  \
    if(ret != MPG123_OK) \
    {                    \
        fprintf(stderr,"[%s in %s] err message:%s",__FUNCTION__,__LINE__,mpg123_plain_strerror(ret)); \
        return false;    \
    }

Mp3FileReader::Mp3FileReader( const char*filename )
{
    int ret;
    ret = mpg123_init();
    if ( ret != MPG123_OK )
    {
        fprintf( stderr, "mpg123_init failed: %s\n", mpg123_plain_strerror( ret ) );
        return;
    }
    m_filename = filename;
    OpenFile( filename, false );

    m_cirbuf = new CircularAudioBuffer( 8192 );

    m_bInit = true;
}

Mp3FileReader::~Mp3FileReader()
{
    mpg123_exit();
}

void Mp3FileReader::Destroy()
{
    Close();
    delete this;
}

int Mp3FileReader::SampleRate() const
{
    return m_nSamplerate;
}

size_t Mp3FileReader::NumChannels() const
{
    return m_nChannels;
}

size_t Mp3FileReader::NumSamples() const
{
    return m_NumSamples;
}

size_t Mp3FileReader::ReadSamples( size_t /* num_samples */ , float* /* samples*/ )
{
    return 0;
//     if (!m_bInit)
//     {
//         OpenFile( m_filename.c_str(),true );
//         m_bInit = true;
//     }
//     if ( m_wavformat != WAVE_FORMAT_IEEE_FLOAT )
//     {
//         return 0;
//     }
//     if ( m_SampleRemain == 0 )
//     {
//         return 0;
//     }
//     int ret = MPG123_OK;
//     size_t need_samples = num_samples;
//     size_t readlen = 0;
//     if ( m_fcirbuf )
//     {
//         readlen = ( std::min )( need_samples, m_fcirbuf->readSizeRemaining() );
//         readlen = m_fcirbuf->read( samples, readlen );
//     }
// 
//     need_samples -= readlen;
//     m_SampleRemain -= readlen;
//     while ( need_samples > 0 )
//     {
//         off_t curFrame;
//         unsigned char* audio = 0;
//         size_t bytes = 0;
//         ret = mpg123_decode_frame( m_hFile, &curFrame, &audio, &bytes );
//         if ( ret == MPG123_DONE )
//         {
//             break;
//         }
// 
//         if ( audio && bytes > 0 )
//         {
//             size_t read = ( std::min )( need_samples, bytes / sizeof(float) );
//             memcpy( samples + readlen, audio, read * sizeof(float) );
//             m_SampleRemain -= read;
//             need_samples -= read;
//             if ( need_samples == 0 )
//             {
//                 if ( m_fcirbuf == nullptr && curFrame > 0 )
//                 {
//                     m_fcirbuf = new CircularBuffer<float>( bytes * 2 );
//                 }
//                 m_fcirbuf->write( (float*)( audio + read * sizeof( float ) ), bytes / sizeof( float ) - read );
//                 break;
//             }
//         }
//         else
//         {
//             break;
//         }
//     }
// 
//     return num_samples - need_samples;
}

size_t Mp3FileReader::ReadSamples( size_t num_samples, int16_t* samples )
{
    if (!m_bInit)
    {
        return 0;
    }

    if (m_wavformat != WAVE_FORMAT_PCM)
    {
        return 0;
    }

    if (m_SampleRemain == 0)
    {
        return 0;
    }
    int ret = MPG123_OK;
    size_t need_samples = num_samples;
    size_t readlen = 0;
    if ( m_cirbuf )
    {
        readlen = ( std::min )( need_samples, m_cirbuf->readSizeRemaining() );
        readlen = m_cirbuf->read( samples, readlen );
    }

    need_samples -= readlen;
    m_SampleRemain -= readlen;
    while ( need_samples > 0 )
    {
        off_t curFrame;
        unsigned char* audio = 0;
        size_t bytes = 0;
        ret = mpg123_decode_frame( m_hFile, &curFrame, &audio, &bytes );
        if (ret == MPG123_DONE)
        {
            break;
        }

        if ( audio && bytes > 0 )
        {
            size_t read = (std::min)( need_samples, bytes / 2 );
            memcpy( samples + readlen, audio, read*2 );
            m_SampleRemain -= read;
            need_samples -= read;
            readlen += read;
            if ( need_samples == 0 )
            {
                m_cirbuf->write( (int16_t*)( audio + read * 2 ), bytes/2 - read );
                break;
            }
        }
        else
        {
            break;
        }
    }

    return num_samples - need_samples;
}

bool Mp3FileReader::OpenFile( const char*filename, bool m_bFloatFormat )
{
    off_t* offsets;
    off_t step;
    size_t fill;
    int ret = MPG123_OK;
    m_hFile = mpg123_new( NULL, NULL );
    if (m_hFile == nullptr)
    {
        fprintf( stderr, "mpg123_new failed\n" );
        return false;
    }

    ret = mpg123_param( m_hFile, MPG123_RESYNC_LIMIT, -1, 0 );
    if (ret != MPG123_OK)
    {
        fprintf( stderr, "mpg123_param failed: %s\n", mpg123_plain_strerror( ret ) );
        return false;
    }
    ret = mpg123_param( m_hFile, MPG123_INDEX_SIZE, -1, 0 );
    if (ret != MPG123_OK)
    {
        fprintf( stderr, "mpg123_param failed: %s\n", mpg123_plain_strerror( ret ) );
        return false;
    }
    if (m_Speed != 0)
    {
        SetSpeed( m_Speed );
    }
    if ( m_bFloatFormat )
    {
        ret = mpg123_format_none(m_hFile);
        if (ret == MPG123_OK)
        {
            size_t nrates = 0;
            const long *rates = nullptr;
            mpg123_rates( &rates, &nrates );
            for ( size_t i = 0; i < nrates; i++ )
            {
                ret = mpg123_format( m_hFile, rates[i], MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32 );
                if ( ret != MPG123_OK )
                {
                    fprintf( stderr, "Unable to set float output formats: %s\n", mpg123_plain_strerror( ret ) );
                    return false;
                }
            }
        }
    }

    ret = mpg123_open( m_hFile, filename );
    if (ret != MPG123_OK)
    {
        fprintf( stderr, "mpg123_param failed: %s\n", mpg123_plain_strerror( ret ) );
        return false;
    }
    ret = mpg123_scan( m_hFile );
    if (ret != MPG123_OK)
    {
        fprintf( stderr, "mpg123_scan failed: %s\n", mpg123_strerror(m_hFile) );
        return false;
    }
    ret = mpg123_index( m_hFile, &offsets, &step, &fill );
    if (ret != MPG123_OK)
    {
        fprintf( stderr, "mpg123_scan failed: %s\n", mpg123_strerror( m_hFile ) );
        return false;
    }

    mpg123_getformat( m_hFile, &m_nSamplerate, &m_nChannels, &m_EncodeFormat);
    InitWavFormat( m_EncodeFormat );
    int length = mpg123_length( m_hFile );
    m_NumSamples = length / m_bitspersample * 8;
    m_SampleRemain = m_NumSamples;
    return true;
}



// determine correct wav format and bits per sample
// from mpg123 enc value
void Mp3FileReader::InitWavFormat(int enc)
{
    if ( enc & MPG123_ENC_FLOAT_64 )
    {
        m_bitspersample = 64;
        m_wavformat = WAVE_FORMAT_IEEE_FLOAT;
    }
    else if ( enc & MPG123_ENC_FLOAT_32 )
    {
        m_bitspersample = 32;
        m_wavformat = WAVE_FORMAT_IEEE_FLOAT;
    }
    else if ( enc & MPG123_ENC_16 )
    {
        m_bitspersample = 16;
        m_wavformat = WAVE_FORMAT_PCM;
    }
    else
    {
        m_bitspersample = 8;
        m_wavformat = WAVE_FORMAT_PCM;
    }
}

void Mp3FileReader::Close()
{
    if (!m_bInit)
    {
        return ;
    }

    mpg123_close( m_hFile );
    mpg123_delete( m_hFile );
    m_hFile = nullptr;
    m_bInit = false;
    delete m_fcirbuf;
    m_fcirbuf = nullptr;
    delete m_cirbuf;
    m_cirbuf = nullptr;
}

bool Mp3FileReader::SeekSamples( size_t pos )
{
    if ( !m_hFile )
    {
        return false;
    }
    off_t frame = mpg123_seek( m_hFile, (off_t)pos, SEEK_SET);
    return mpg123_seek_frame( m_hFile, frame, SEEK_SET ) > 0;
}

bool Mp3FileReader::SeekTime( double time )
{
    if (!m_hFile)
    {
        return false;
    }
    off_t frame = mpg123_timeframe( m_hFile, time );
    return mpg123_seek_frame( m_hFile, frame, SEEK_SET ) > 0;
}

size_t Mp3FileReader::RemainSamples() const
{
    return m_SampleRemain;
}

bool Mp3FileReader::SetSpeed( double times )
{
    if (m_hFile == nullptr)
    {
        m_Speed = times;
        return true;
    }
    if (times> 1)
    {
        int ret = mpg123_param( m_hFile, MPG123_UPSPEED, (int)times, 0 );
        if ( ret != MPG123_OK )
        {
            fprintf( stderr, "mpg123_param  MPG123_UPSPEED failed: %s\n", mpg123_plain_strerror( ret ) );
            return false;
        }
    }
    else
    {
        int ret = mpg123_param( m_hFile, MPG123_DOWNSPEED, (int)(1./times), 0 );
        if ( ret != MPG123_OK )
        {
            fprintf( stderr, "mpg123_param  MPG123_DOWNSPEED failed: %s\n", mpg123_plain_strerror( ret ) );
            return false;
        }
    }
    return true;
}



