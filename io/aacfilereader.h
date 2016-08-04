#pragma once
#include "io/include/audioreader.h"
#include "codec/aac/libAACdec/include/aacdecoder_lib.h"
#include <stdio.h>
class AACFileReader :public AudioReader
{
public:
    AACFileReader(const char* filename );
    ~AACFileReader();
    virtual void   Destroy();
    virtual int    SampleRate() const;
    virtual size_t NumChannels() const;
    virtual size_t NumSamples() const;
    virtual size_t ReadSamples( size_t num_samples, float* samples );
    virtual size_t ReadSamples( size_t num_samples, int16_t* samples );
    virtual size_t RemainSamples()const;
    virtual bool   SeekSamples( size_t pos );
    virtual bool   SeekTime( double sec );
    virtual bool   SetSpeed( double times );
private:
    bool ReadFrame();
    HANDLE_AACDECODER aacDecoderInfo = nullptr;
    FILE* m_file = nullptr;
    bool m_bInit= false;
    long m_nSamplerate = 0;
    int m_nChannels = 0;
    int m_nSample = 0;
    int m_readSample = 0;
    char m_inBuf[512];
    INT_PCM m_outBuf[4096];
    int m_advace_pos = 4096;
    UINT bytevalid = 0;
};