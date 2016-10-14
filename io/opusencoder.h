#pragma once
#include "include/audioencoder.h"
#include "codec/opus/include/opus.h"
class OPUSEncoder :public AudioEncoder
{
public:
    OPUSEncoder(int samplerate,int16_t channel,int bitrate);
    ~OPUSEncoder();
    virtual void Release();
    virtual bool SetBitRate( int32_t bitRate );
    virtual bool Encode( int16_t* pcmData, int nSamples, char* encodeData, int& outLen );
private:
    OpusEncoder *_enc = nullptr;
    int _samplerate;
    int _channel;
};