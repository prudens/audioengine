#pragma once
#include "include/audiodecoder.h"
#include "codec/aac/libAACdec/include/aacdecoder_lib.h"
class AACDecoder :public AudioDecoder
{
    static const int INPUT_BUF_SIZE = 2 * 768;
    static const int OUTPUT_BUF_SIZE = 4096;
public:
    AACDecoder();
    ~AACDecoder();
    virtual void Release()override;
    virtual bool Decode( void* encodeData, int outLen, int16_t* pcmData, int& inLen ) override;
private:
    HANDLE_AACDECODER m_hAACDecoder = nullptr;
    bool    m_bInit = false;
    long    m_nSamplerate = 0;
    int     m_nChannels = 0;
    size_t  m_nSample = 0;
    size_t  m_readSample = 0;
    UCHAR   m_inBuf[INPUT_BUF_SIZE];
    INT_PCM m_outBuf[OUTPUT_BUF_SIZE];
    int     m_advace_pos = OUTPUT_BUF_SIZE;
    UINT    m_bytevalid = 0;
    int     m_frameSize = OUTPUT_BUF_SIZE;
};