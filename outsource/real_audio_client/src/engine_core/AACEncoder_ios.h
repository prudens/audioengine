//
//  AACCoder.h
//  audio_engine
//
//  Created by 陳偉榮 on 2017/6/2.
//  Copyright © 2017年 snailgame. All rights reserved.
//

#ifndef AACCoder_h
#define AACCoder_h
#ifdef _IOS
#include <cstdint>
#import<AudioToolbox/AudioToolbox.h>
#include <list>
#include "codec_converter.h"
#define MAX_FRAME_SIZE 1024*8

class AACEncoderIOS:public CodecConverter
{
public:
    AACEncoderIOS();
    virtual ~AACEncoderIOS();
    bool Init( int aacObjectType, int samplerate, int channel,int bitrate );
    virtual int Process( const void* inputData, size_t inputSize, void* outputData, size_t& outputSize );
    virtual void Destroy();
public:
    bool Encode(int16_t* pcm,size_t sizeInbyte, void* &encData,size_t& len );
    AudioClassDescription* getAudioClassDescription(UInt32 type, UInt32 manufacturer );
    int GetAACFormatID(int aacObjectType);
    int m_samplerate = 48000;
    int m_channel = 2;
    AudioConverterRef _audioConverter = NULL;
    char*    m_cur_frame              = nullptr;
    char*    m_prev_frame             = nullptr;// 系统API要求至少要把pcm数据保存一帧。
    UInt32   m_inputDataSize          = MAX_FRAME_SIZE;
    UInt32   m_maxOutputPackageSize   = 0;
    int      m_err                    = 0;

};
#endif
#endif /* AACCoder_h */
