//
//  AACCoder.c
//  audio_engine
//
//  Created by 陳偉榮 on 2017/6/2.
//  Copyright © 2017年 snailgame. All rights reserved.
//
#ifdef _IOS
#include "AACEncoder_IOS.h"
#import<Foundation/Foundation.h>
#include <string>
#include <algorithm>
    

static OSStatus inInputDataProc( AudioConverterRef               inAudioConverter,
                      UInt32 *                        ioNumberDataPackets,
                      AudioBufferList *               ioData,
                      AudioStreamPacketDescription * __nullable * __nullable outDataPacketDescription,
                     void * __nullable               inUserData)
{
    AACEncoderIOS *encoder = ( AACEncoderIOS *)(inUserData);
    ioData->mBuffers[0].mData = encoder->m_cur_frame;
    ioData->mBuffers[0].mDataByteSize = encoder->m_inputDataSize;
    ioData->mBuffers[0].mNumberChannels = 2;
    ioData->mNumberBuffers = 1;
    *ioNumberDataPackets = encoder->m_inputDataSize/4;
    return noErr;
}

AACEncoderIOS::AACEncoderIOS()
{
}

AACEncoderIOS::~AACEncoderIOS()
{
    OSStatus status = AudioConverterDispose(_audioConverter);
    if (status != 0 )
    {
        NSLog(@"destroy audio converter failed:%d\n",(int)status);
    }
    
    if (m_prev_frame)
    {
        delete m_prev_frame;
    }
    
    if (m_cur_frame)
    {
        delete m_cur_frame;
    }
}

bool AACEncoderIOS::Init( int aacObjectType, int samplerate, int channel,int bitrate )
{
    int formatid = GetAACFormatID(aacObjectType);
    if (formatid == -1) {
        return false;
    }
    OSStatus status = 0;
    m_samplerate = samplerate;
    m_channel = channel;
    int kBytePerSample = channel * 2;
    AudioStreamBasicDescription inAudioStreamBasicDescription;
    inAudioStreamBasicDescription.mFormatID = kAudioFormatLinearPCM;
    inAudioStreamBasicDescription.mSampleRate = samplerate;
    inAudioStreamBasicDescription.mBitsPerChannel = 16;
    inAudioStreamBasicDescription.mFramesPerPacket = 1;
    inAudioStreamBasicDescription.mChannelsPerFrame = channel;
    inAudioStreamBasicDescription.mBytesPerFrame = kBytePerSample;
    inAudioStreamBasicDescription.mBytesPerPacket = kBytePerSample;

    inAudioStreamBasicDescription.mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsSignedInteger;
    inAudioStreamBasicDescription.mReserved = 0;
    
    AudioStreamBasicDescription outAudioStreamBasicDescription = {0}; // Always initialize the fields of a new audio stream basic description structure to zero, as shown here: ...
    outAudioStreamBasicDescription.mChannelsPerFrame = channel;
    outAudioStreamBasicDescription.mSampleRate = samplerate;
    outAudioStreamBasicDescription.mFormatID = formatid;
    UInt32 size = sizeof(outAudioStreamBasicDescription);
    status = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &outAudioStreamBasicDescription);
    
    status = AudioConverterNew(&inAudioStreamBasicDescription, &outAudioStreamBasicDescription, &_audioConverter);
    if(status!=0)
    {
        NSLog(@"AudioConverterNew converter failed: %d\n",(int)status);
        m_err = status;
        return false;
    }

    AudioValueRange* r;
    UInt32 ulSize = 0;
    UInt32 ulBitRate = 0;
    
    
    status = AudioConverterGetPropertyInfo(_audioConverter,kAudioConverterApplicableEncodeBitRates,&ulSize,nullptr);
    if(status != 0)
    {
        NSLog(@"[snail_audio_client] AACEncoder:Can not find a valid ApplicableEncodeBitRates,ec=%d \n",(int)status);
        m_err = status;
        return false;
    }
    r = new AudioValueRange[ulSize/sizeof(AudioValueRange)];
    status = AudioConverterGetProperty(_audioConverter,kAudioConverterApplicableEncodeBitRates, &ulSize, r);
    if (status)
    {
        NSLog(@"[snail_audio_client] AACEncoder:Can not find a valid ApplicableEncodeBitRates2,ec=%d \n",(int)status);
        m_err = status;
        return false;
    }
    for (int i = 0; i < ulSize/sizeof(AudioValueRange); i++)
    {
        if(r[i].mMinimum >= bitrate)
        {
            bitrate = r[i].mMinimum;
            break;
        }
    }
    //可能没有找到？
    
    ulBitRate = bitrate;
    ulSize = sizeof(ulBitRate);
    status = AudioConverterSetProperty(_audioConverter, kAudioConverterEncodeBitRate, ulSize, &ulBitRate);
    if(status != 0)
    {
        NSLog(@"[snail_audio_client] AudioConverterSetProperty set bitrate (%d) failed: %d\n",bitrate,(int)status);
        m_err = status;
        return false;
    }
    
    size = sizeof(m_maxOutputPackageSize);
    status = AudioConverterGetProperty(_audioConverter, kAudioConverterPropertyMaximumOutputPacketSize, &size, &m_maxOutputPackageSize);
    if(status == 0 )
    {
      //  NSLog(@"[snail_audio_client] MaximumOutputPacketSize=%d\n", (int)m_maxOutputPackageSize);
    }
    else
    {
        NSLog(@"[snail_audio_client] Get MaximumOutputPacketSize failed: %d\n", (int)status);
        m_err = status;
        return  false;
    }
    
    return true;
}

bool AACEncoderIOS::Encode(int16_t *pcm, size_t sizeInbyte, void* &encData,size_t& len)
{
    if (!_audioConverter || m_maxOutputPackageSize == 0)
    {
        return false;
    }
    
    m_inputDataSize = sizeInbyte;
    if (m_cur_frame == nullptr)
    {
        m_cur_frame = new char[sizeInbyte];
        m_prev_frame = new char[sizeInbyte];
    }

    memcpy(m_cur_frame, pcm, m_inputDataSize);

    AudioBufferList outAudioBufferList = {0};
    outAudioBufferList.mNumberBuffers = 1;
    outAudioBufferList.mBuffers[0].mNumberChannels = 2;
    outAudioBufferList.mBuffers[0].mDataByteSize = len;
    outAudioBufferList.mBuffers[0].mData = encData;
    
    UInt32 ioOutputDataPacketSize = 1;
    OSStatus status = AudioConverterFillComplexBuffer(_audioConverter, inInputDataProc, this, &ioOutputDataPacketSize, &outAudioBufferList, NULL);
    std::swap(m_cur_frame,m_prev_frame);
    
    if ( status != 0 )
    {
        NSLog(@"AudioConverterFillComplexBuffer failed:%d\n",(int)status);
        len = 0;
        m_err = status;
        return false;
    }
    
    len = outAudioBufferList.mBuffers[0].mDataByteSize;

    return true;
}

int AACEncoderIOS::Process( const void* inputData, std::size_t inputSize, void* outputData, size_t& outputSize )
{
    if (Encode((int16_t*)inputData, inputSize, outputData, outputSize))
    {
        return 0;
    }
    else
    {
        return m_err;
    }
}
void AACEncoderIOS::Destroy()
{
    delete this;
}

/*
kAudioFormatMPEG4AAC                = 'aac ',
kAudioFormatMPEG4AAC_HE             = 'aach',
kAudioFormatMPEG4AAC_LD             = 'aacl',
kAudioFormatMPEG4AAC_ELD            = 'aace',
kAudioFormatMPEG4AAC_ELD_SBR        = 'aacf',
kAudioFormatMPEG4AAC_ELD_V2         = 'aacg',
kAudioFormatMPEG4AAC_HE_V2          = 'aacp',
kAudioFormatMPEG4AAC_Spatial        = 'aacs',
 */

int AACEncoderIOS::GetAACFormatID(int aacObjectType)
{
    switch (aacObjectType)
    {
        case AACObjectLC:
            return kAudioFormatMPEG4AAC;
        case AACObjectHE:
            return kAudioFormatMPEG4AAC_HE;
        case AACObjectHE_PS:
            return kAudioFormatMPEG4AAC_HE_V2;
        case AACObjectLD:
            return kAudioFormatMPEG4AAC_LD;
        case AACObjectELD:
            return kAudioFormatMPEG4AAC_ELD;
        default:
            return -1;
    }
}

AudioClassDescription* AACEncoderIOS::getAudioClassDescription(UInt32 type, UInt32 manufacturer )
{
    static AudioClassDescription desc;
    
    UInt32 encoderSpecifier = type;
    OSStatus st;
    
    UInt32 size;
    st = AudioFormatGetPropertyInfo(kAudioFormatProperty_Encoders,
                                    sizeof(encoderSpecifier),
                                    &encoderSpecifier,
                                    &size);
    if (st) {
        NSLog(@"error getting audio format propery info: %d", (int)(st));
        return nil;
    }
    
    unsigned int count = size / sizeof(AudioClassDescription);
    AudioClassDescription descriptions[count];
    st = AudioFormatGetProperty(kAudioFormatProperty_Encoders,
                                sizeof(encoderSpecifier),
                                &encoderSpecifier,
                                &size,
                                descriptions);
    if (st) {
        NSLog(@"error getting audio format propery: %d", (int)(st));
        return nil;
    }
    
    for (unsigned int i = 0; i < count; i++) {
        if ((type == descriptions[i].mSubType) &&
            (manufacturer == descriptions[i].mManufacturer)) {
            memcpy(&desc, &(descriptions[i]), sizeof(desc));
            return &desc;
        }
    }
    
    return nil;
}

#endif
