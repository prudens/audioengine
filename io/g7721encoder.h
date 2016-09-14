#pragma once
#include "./include/audioencoder.h"
#include "codec/g7221/include/defs.h"
class G7221Encoder :public AudioEncoder
{
public:
    G7221Encoder();
    virtual void Release();
    virtual bool Encode( int16_t* pcmData, int inLen, void* encodeData, int& outLen )override;
private:
    bool DoEncode( int16_t* pcmData, int inLen, void* encodeData, int& outLen );
    int number_of_regions = 28;
    int sample_rate = 16000;
    int bit_rate = 16000;
    int number_of_bits_per_frame = bit_rate / 25;//40ms
    int number_of_16bit_words_per_frame = number_of_bits_per_frame / 16;
    int framesize = sample_rate / 25; // 40ms
    float mlt_coefs[MAX_DCT_SIZE];
    float float_new_samples[MAX_DCT_SIZE];
    int need_size = framesize;
    int16_t input[MAX_DCT_SIZE];
};