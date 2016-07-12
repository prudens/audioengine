#pragma once
#include <cstdint>
#include "io_common_define.h"
class AudioWriter
{
public:
    static AudioWriter*Create( const char* filename, int sample_rate, size_t channels, AudioFileType type );
    virtual int SampleRate() const = 0;
    virtual size_t NumChannels() const = 0;
    virtual size_t NumSamples() const = 0;
    virtual void WriteSamples( const float* samples, size_t num_samples ) = 0;
    virtual void WriteSamples( const int16_t* samples, size_t num_samples ) = 0;
};