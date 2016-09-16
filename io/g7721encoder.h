#pragma once
#include "./include/audioencoder.h"
#include "codec/g7221/include/g722_1.h"
#define  MAX_PCM_FRAME_SIZE 640
class G7221Encoder :public AudioEncoder
{
public:
    G7221Encoder( int32_t samplerate, int16_t channel );
    virtual void Release();
    virtual bool Encode( int16_t* pcmData, int inLen, void* encodeData, int& outLen )override;
    virtual bool SetBitRate( int32_t bitRate );
private:
    int16_t m_pcm[MAX_FRAME_SIZE];
    uint8_t  m_encbuf[MAX_BITS_PER_FRAME / 8];
    g722_1_encode_state_t *m_encoder = nullptr;
    size_t m_curPos = 0;
    bool m_init = false;
    size_t m_frameSize;
};