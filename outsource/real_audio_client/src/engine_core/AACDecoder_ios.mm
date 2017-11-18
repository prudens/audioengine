//
//  AACDecoder.cpp
//  audio_engine
//
//  Created by 陳偉榮 on 2017/6/5.
//  Copyright © 2017年 snailgame. All rights reserved.
//
#ifdef _IOS
#include "AACDecoder_ios.h"


static OSStatus inInputDataProc(AudioConverterRef aAudioConverter,
                         UInt32* aNumDataPackets /* in/out */,
                         AudioBufferList* aData /* in/out */,
                         AudioStreamPacketDescription** aPacketDesc,
                         void* aUserData)
{
    AACDecoderIOS* userData = (AACDecoderIOS*)aUserData;
    if (userData->m_decframelen == 0) {
        *aNumDataPackets = 0;
        return -1;
    }
    if (aPacketDesc) {
        userData->mPacket.mStartOffset = 0;
        userData->mPacket.mVariableFramesInPacket = 0;
        userData->mPacket.mDataByteSize = userData->m_decframelen;
        *aPacketDesc = &(userData->mPacket);
    }
    aData->mNumberBuffers = 1;
    aData->mBuffers[0].mNumberChannels = userData->m_channel;
    aData->mBuffers[0].mDataByteSize = userData->m_decframelen;
    aData->mBuffers[0].mData = static_cast<void*>(userData->m_decframe);
    *aNumDataPackets = 1;
    return noErr;
}

AACDecoderIOS::AACDecoderIOS()
{

}

AACDecoderIOS::~AACDecoderIOS()
{
    OSStatus status = AudioConverterDispose(_audioConverter);
    if (status != 0 ) {
        NSLog(@"destroy audio converter failed:%d\n",(int)status);
    }
}

bool AACDecoderIOS::Init( int aacObjectType, int samplerate, int channel )
{
    int formatid = GetAACFormatID(aacObjectType);
    m_samplerate = samplerate;
    m_channel = channel;
    int kBytePerSample = channel * 2;
    AudioStreamBasicDescription outFormat;
    memset(&outFormat, 0, sizeof(outFormat));
    outFormat.mSampleRate       = samplerate;
    outFormat.mFormatID         = kAudioFormatLinearPCM;
    outFormat.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger|kLinearPCMFormatFlagIsPacked;
    outFormat.mBytesPerPacket   = kBytePerSample;
    outFormat.mFramesPerPacket  = 1;
    outFormat.mBytesPerFrame    = kBytePerSample;
    outFormat.mChannelsPerFrame = channel;
    outFormat.mBitsPerChannel   = 16;
    outFormat.mReserved         = 0;
    
    
    AudioStreamBasicDescription inFormat;
    memset(&inFormat, 0, sizeof(inFormat));
    inFormat.mSampleRate        = samplerate;
    inFormat.mFormatID          = formatid;
    inFormat.mChannelsPerFrame  = channel;
    UInt32 size = sizeof(AudioStreamBasicDescription);
    UInt32 status = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &inFormat);
    
    status =  AudioConverterNew(&inFormat, &outFormat, &_audioConverter);
    if (status != 0) {
        NSLog(@"setup converter error, status: %i\n", (int)status);
        m_err = status;
    }
     
    return true;
}

bool AACDecoderIOS::Decode(void* data, size_t len,void* encData,size_t& outlen)
{
    if(!_audioConverter){
        return false;
    }
    m_decframe = data;
    m_decframelen = len;
    AudioBufferList decBuffer;
    decBuffer.mNumberBuffers = 1;
    decBuffer.mBuffers[0].mNumberChannels = m_channel;
    decBuffer.mBuffers[0].mDataByteSize = outlen;
    decBuffer.mBuffers[0].mData = encData;
    UInt32 numFrames = MAX_AUDIO_FRAMES1;
    OSStatus rv = AudioConverterFillComplexBuffer(_audioConverter,
                                                    inInputDataProc,
                                                    this,
                                                    &numFrames /* in/out */,
                                                    &decBuffer,
                                                    &mPacket);
    if ( rv == 0 && numFrames == MAX_AUDIO_FRAMES1) {
        outlen = numFrames*2*m_channel;
        return  true;
    }
    else
    {
        NSLog(@"[AACDecoder]AudioConverterFillComplexBuffer failed:%d\n",(int)rv);
        m_err = rv;
        outlen = 0;
        return false;
    }


    return true;
}

int AACDecoderIOS::GetAACFormatID(int aacObjectType)
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

int AACDecoderIOS::Process( const void* inputData, size_t inputSize, void* outputData, size_t& outputSize )
{
    if( Decode((void*)inputData, inputSize, outputData, outputSize))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
void AACDecoderIOS::Destroy()
{
    delete this;
}
#endif
