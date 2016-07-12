#pragma once
#include <cstdint>
#include "io_common_define.h"
class AudioReader
{
public:
    static AudioReader* Create(const char*filename,AudioFileType type = AFT_DEFAULT);
    virtual void   Destroy() = 0;
    virtual int    SampleRate() const = 0;
    virtual size_t NumChannels() const = 0;
    virtual size_t NumSamples() const = 0;
    virtual size_t ReadSamples( size_t num_samples, float* samples ) = 0;
    virtual size_t ReadSamples( size_t num_samples, int16_t* samples ) = 0;
    virtual size_t RemainSamples()const = 0;
    virtual bool   SeekSamples( size_t pos ) = 0;
    virtual bool   SeekTime( double sec ) = 0;
    virtual bool   SetSpeed( double times ) = 0;
protected:
    ~AudioReader() {}
};
