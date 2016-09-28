#pragma once
#include <stdio.h>
#include "io/include/audioreader.h"
#include "codec/aac/libAACdec/include/aacdecoder_lib.h"
class AACFileReader :public AudioReader
{
public:

    static const int INPUT_BUF_SIZE = 2 * 768;
    static const int OUTPUT_BUF_SIZE = 4096;
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
    bool           Initialized() { return m_bInit; }
private:
    bool ReadFrame();
    bool Analyze( int indexFrame = -1, int* pos = nullptr );
private:
    HANDLE_AACDECODER m_hAACDecoder = nullptr;
    FILE*   m_file = nullptr;
    bool    m_bInit= false;
    long    m_nSamplerate = 0;
    int     m_nChannels = 0;
    size_t  m_nSample = 0;
    size_t  m_readSample = 0;
    UCHAR   m_inBuf[INPUT_BUF_SIZE];
    INT_PCM m_outBuf[OUTPUT_BUF_SIZE];
    int     m_advace_pos = OUTPUT_BUF_SIZE;
    UINT    m_bytevalid = 0;
    int     m_frameSize = OUTPUT_BUF_SIZE/2;
};