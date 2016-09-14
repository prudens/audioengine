#pragma once
#include "./include/audiodecoder.h"
#include "codec/g7221/include/defs.h"
class G7221Decoder :public AudioDecoder
{
public:
    G7221Decoder();
    virtual void Release();
    virtual bool Decode( void* encodeData, int outLen, int16_t* pcmData, int& inLen )override;
private:
    bool DoDecode( void* encodeData, int& outLen, int16_t* pcmData, int& inLen );
    int number_of_regions = 14;
    int sample_rate = 16000;
    int bit_rate = 16000;
    int number_of_bits_per_frame = bit_rate / 50;
    int number_of_16bit_words_per_frame = number_of_bits_per_frame / 16;
    int framesize = sample_rate / 50;
    float mlt_coefs[MAX_DCT_SIZE];
    float float_new_samples[MAX_DCT_SIZE];
    int need_size = framesize;
    int16_t input[MAX_DCT_SIZE];
    int frame_error_flag = 0;
};