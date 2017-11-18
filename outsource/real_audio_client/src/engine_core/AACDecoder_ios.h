//
//  AACDecoder.hpp
//  audio_engine
//
//  Created by 陳偉榮 on 2017/6/5.
//  Copyright © 2017年 snailgame. All rights reserved.
//

#ifndef AACDecoder_hpp
#define AACDecoder_hpp
#ifdef _IOS
#import<AudioToolbox/AudioToolbox.h>
#include <list>
#include "codec_converter.h"
#define MAX_AUDIO_FRAMES1 1024*2
class AACDecoderIOS:public CodecConverter
{
public:
    AACDecoderIOS();
    ~AACDecoderIOS();
    virtual int Process( const void* inputData, size_t inputSize, void* outputData, size_t& outputSize );
    virtual void Destroy();
public:
    bool Init( int aacObjectType, int samplerate, int channel );
    bool Decode(void* data, size_t len, void* encData, size_t& outlen);
    int GetAACFormatID(int aacObjectType);
    AudioConverterRef _audioConverter;
    int m_samplerate = 48000;
    int m_channel = 2;
    void* m_decframe = nullptr;
    int m_decframelen = 0;
    AudioStreamPacketDescription mPacket;
    int m_err = 0;
};

#endif


#endif /* AACDecoder_hpp */
