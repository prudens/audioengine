#pragma once
#include "io/include/audioreader.h"
#ifdef _WIN32
#include "codec/mp3/decoder/ports/MSVC++/mpg123.h"
#else
#include "codec/mp3/decoder/src/libmpg123/mpg123.h"
#endif
#include "base/circular_buffer.hpp"
#include <string>
class Mp3FileReader : public AudioReader
{
public:
    enum
    {
        WAVE_FORMAT_PCM                     =   0x0001,
        WAVE_FORMAT_IEEE_FLOAT              =   0x0003, /* Microsoft Corporation */
    };
    Mp3FileReader( const char* filename );
    virtual ~Mp3FileReader();
    virtual void   Destroy()override;
    virtual int    SampleRate() const override;
    virtual size_t NumChannels() const override;
    virtual size_t NumSamples() const override;
    virtual size_t ReadSamples( size_t num_samples, float* samples ) override;
    virtual size_t ReadSamples( size_t num_samples, int16_t* samples ) override;
    virtual size_t RemainSamples()const override;
    virtual bool   SeekSamples( size_t pos ) override;
    virtual bool   SeekTime( double time ) override;
    virtual bool   SetSpeed( double times ) override;
private:
    bool OpenFile( const char*filename, bool m_bFloatFormat );
    void Close();
    void InitWavFormat(int enc);
private:
    mpg123_handle *m_hFile = nullptr;
    long m_nSamplerate = 0;
    int m_nChannels = 0;
    int m_EncodeFormat = 0;
    int m_bitspersample = 16;
    int m_wavformat = WAVE_FORMAT_PCM;
    size_t m_NumSamples = 0;
    size_t  m_SampleRemain = 0;
    CircularAudioBuffer* m_cirbuf;
    CircularBuffer<float>* m_fcirbuf;
    bool m_bInit = false;
    std::string m_filename;
    double m_Speed;
};