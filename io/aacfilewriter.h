#pragma once
#include <stdio.h>
#include "io/include/audiowriter.h"
#include "codec/aac/libAACenc/include/aacenc_lib.h"
#include "base/circular_buffer.hpp"
class AACFileWriter :public AudioWriter
{
public:
    AACFileWriter(const char* filename, int samplerate,int channel);
    virtual ~AACFileWriter();
    virtual void Destroy()override;
    virtual int SampleRate() const override;
    virtual size_t NumChannels() const override;
    virtual size_t NumSamples() const override;
    virtual void WriteSamples( const float* samples, size_t num_samples ) override;
    virtual void WriteSamples( const int16_t* samples, size_t num_samples ) override;
private:
    HANDLE_AACENCODER m_hAacEncoder = nullptr;
    bool m_bInit = false;
    int m_samplerate = 44100;
    int m_channel = 2;
    int m_nSamples = 0;
    FILE* m_aacfile = nullptr;

    int m_framesize;

    AACENC_BufDesc m_encinBuf;
    int m_in_eisize = 2;
    int m_in_bufsize;
    int m_in_buf_id = IN_AUDIO_DATA;

    AACENC_BufDesc m_encoutBuf;
    int m_out_eisize = 2;
    int m_out_bufsize;
    int m_out_buf_id = OUT_BITSTREAM_DATA;


    AACENC_InArgs m_in_args;
    AACENC_OutArgs m_out_args;
    char* m_outofbyte;
    int m_advance_samples = 0;
    char* m_pInputbuf = nullptr;
};