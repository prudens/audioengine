#pragma once
#include "include/audiodecoder.h"
#include "codec/opus/include/opus.h"
class OPUSDecoder :public AudioDecoder
{
public:
    OPUSDecoder(int samplerate,int channel);
    ~OPUSDecoder();
    virtual void Release();
    virtual bool Decode( void* encodeData, int outLen, int16_t* pcmData, int& nSmaples );
private:
    OpusDecoder *dec = nullptr;
    int use_inbandfec = 1;
    int count = 0;
    int lost_prev = 0;
    unsigned char prevData[1500];
    int prevLen = 0;
    int samplerate;
    int channel;
};