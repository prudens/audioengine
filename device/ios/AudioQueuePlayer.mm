//
//  AudioQueuePlayer.cpp
//  audio_engine
//
//  Created by 陳偉榮 on 16/4/5.
//  Copyright © 2016年 snailgame. All rights reserved.
//

#include "AudioQueuePlayer.h"
#include <sys/utsname.h>
#include <UIKit/UIDevice.h>
const char* GetIOSDeviceInfo()
{
    static char achDeviceInfo[128]={0};
	if(strlen(achDeviceInfo)>0)
	{
	    return achDeviceInfo;
	}
    struct utsname systemInfo;
    uname(&systemInfo);
    sprintf(achDeviceInfo, "c:%s|id:%s|v:%s",[[[UIDevice currentDevice] systemName] cStringUsingEncoding:NSASCIIStringEncoding],systemInfo.machine,[[[UIDevice currentDevice] systemVersion] cStringUsingEncoding:NSASCIIStringEncoding]);
    int len = strlen(achDeviceInfo);
	for(int i = 0; i < len; i++)
	{
	    if(achDeviceInfo[i] >= 'A' && achDeviceInfo[i] <= 'Z')
		{
		    achDeviceInfo[i] += 0x20;
		}
	}
	return achDeviceInfo;
}

static void InterruptionListenerCallback(void *inUserData, UInt32 interruptionState)
{
    /*
    CAudioQueuePlayer *player = ( CAudioQueuePlayer *)inUserData;
    if (interruptionState == kAudioSessionBeginInterruption)
    {
        player->tearDownPlayAudio();
        player->tearDownRecordAudio();
    }
    else if (interruptionState == kAudioSessionEndInterruption)
    {
        player->setUpPlayAudio();
        player->StartPlay();
        player->setUpRecordAudio();
        player->StartRecord();
    }
     */
}


static void PlayCallback(void *inUserData, AudioQueueRef inAudioQueue, AudioQueueBufferRef inBuffer)
{
    CAudioQueuePlayer *player = ( CAudioQueuePlayer *)inUserData;
    if (player->playing && player->playblock != nullptr && inAudioQueue && inBuffer)
    {
        player->playblock(inBuffer, player->playaudioFormat);
        AudioQueueEnqueueBuffer(inAudioQueue, inBuffer, 0, NULL);
         
    }
}



static void RecordCallback(void *inUserData,
                                AudioQueueRef inAQ,
                                AudioQueueBufferRef inBuffer,
                                const AudioTimeStamp *inTimeStamp,
                                UInt32 inNumPackets,
                                const AudioStreamPacketDescription *inPacketDesc)
{
    CAudioQueuePlayer *aqr = ( CAudioQueuePlayer *)inUserData;


        if(inNumPackets > 0){
            
            // 后台保存数据到自己的数据队列中
            if(aqr->recordblock) aqr->recordblock(inBuffer,aqr->recordaudioFormat);
            AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
        }
}

CAudioQueuePlayer::CAudioQueuePlayer()
{
    
}

void CAudioQueuePlayer::InitPlayout(int sampleRate, int channels, int bitsPerChannel,int packetsPerBuffer)
{
    playing = false;
    _playQueue = NULL;
    gain = 1.0;
    
    playaudioFormat.mFormatID         = kAudioFormatLinearPCM;
    playaudioFormat.mSampleRate       = sampleRate;
    playaudioFormat.mChannelsPerFrame = channels;
    playaudioFormat.mBitsPerChannel   = bitsPerChannel;
    playaudioFormat.mFramesPerPacket  = 1;  // uncompressed audio
    playaudioFormat.mBytesPerFrame    = playaudioFormat.mChannelsPerFrame * playaudioFormat.mBitsPerChannel/8;
    playaudioFormat.mBytesPerPacket   = playaudioFormat.mBytesPerFrame * playaudioFormat.mFramesPerPacket;
    playaudioFormat.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    
    _playpacketsPerBuffer = packetsPerBuffer;
    _playbytesPerBuffer = _playpacketsPerBuffer * playaudioFormat.mBytesPerPacket;
    setUpPlayAudio();
}


void CAudioQueuePlayer::InitRecord(int sampleRate, int channels, int bitsPerChannel,int packetsPerBuffer)
{
    recording = false;
    _recordQueue = NULL;
    
    recordaudioFormat.mFormatID         = kAudioFormatLinearPCM;//kAudioFormatMPEG4AAC_HE_V2;//kAudioFormatMPEG4AAC_HE;//
    recordaudioFormat.mSampleRate       = sampleRate;
    recordaudioFormat.mChannelsPerFrame = channels;
    recordaudioFormat.mBitsPerChannel   = bitsPerChannel;
    recordaudioFormat.mFramesPerPacket  = 1;  // uncompressed audio
    recordaudioFormat.mBytesPerFrame    = recordaudioFormat.mChannelsPerFrame * recordaudioFormat.mBitsPerChannel/8;
    recordaudioFormat.mBytesPerPacket   = recordaudioFormat.mBytesPerFrame * recordaudioFormat.mFramesPerPacket;
    recordaudioFormat.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    
    _recordpacketsPerBuffer             = packetsPerBuffer;
    _recordbytesPerBuffer               = _recordpacketsPerBuffer * recordaudioFormat.mBytesPerPacket;
    setUpRecordAudio();
}

void CAudioQueuePlayer::StartPlay()
{
    if (!playing)
    {
        playing = true;
        primePlayQueueBuffers();
        AudioQueueStart(_playQueue, NULL);
    }
}


void CAudioQueuePlayer::StartRecord()
{
    if (!recording)
    {
        recording = true;
        primeRecordQueueBuffers();
        OSStatus ost = AudioQueueStart(_recordQueue, NULL);
        printf("AudioQueueStart,ost:%d\n",ost);
    }
}

void CAudioQueuePlayer::StopPlay()
{
    if (playing)
    {
        AudioQueueStop(_playQueue, TRUE);
        playing = false;
    }
}


void CAudioQueuePlayer::StopRecord()
{
    if (recording)
    {
        AudioQueueStop(_recordQueue, TRUE);
        recording = false;
    }
}


void CAudioQueuePlayer::setUpPlayAudio()
{
    if (_playQueue == NULL)
    {
        this->setUpAudioSession();
        this->InitPlayQueue();
        this->setUpPlayQueueBuffers();
    }
}


void CAudioQueuePlayer::setUpRecordAudio()
{
    if (_recordQueue == NULL)
    {
        this->setUpAudioSession();
        this->InitRecordQueue();
        this->setUpRecordQueueBuffers();
    }
}



void CAudioQueuePlayer::tearDownPlayAudio()
{
    if (_playQueue != NULL)
    {
        this->StopPlay();
        this->UnInitPlayQueue();
        this->tearDownAudioSession();
    }
}

void CAudioQueuePlayer::tearDownRecordAudio()
{
    if (_recordQueue != NULL)
    {
        this->StopRecord();
        this->UnInitRecordQueue();
        this->tearDownAudioSession();
    }
}


void CAudioQueuePlayer::setUpAudioSession()
{
    if (playing || recording) {
        return;
    }
    AudioSessionInitialize(
                       NULL,
                       NULL,
                       InterruptionListenerCallback,
                       (void *)this);

    UInt32 sessionCategory = kAudioSessionCategory_PlayAndRecord;
    AudioSessionSetProperty(
                        kAudioSessionProperty_AudioCategory,
                        sizeof(sessionCategory),
                        &sessionCategory);

    //AudioSessionSetActive(true);
}

void CAudioQueuePlayer::tearDownAudioSession()
{
    if (playing||recording) {
        return;
    }
    //AudioSessionSetActive(false);
}

void CAudioQueuePlayer::InitPlayQueue()
{
    AudioQueueNewOutput(
                        &playaudioFormat,
                        PlayCallback,
                        ( void *)this,
                        NULL,                   // run loop
                        kCFRunLoopCommonModes,  // run loop mode
                        0,                      // flags
                        &_playQueue);
    
    gain = 1.0;
}


void CAudioQueuePlayer::InitRecordQueue()
{
    OSStatus ost ;
    ost=AudioQueueNewInput(
                       &recordaudioFormat,
                       RecordCallback,
                       ( void *)this,
                       NULL,                   // run loop
                       kCFRunLoopCommonModes,  // run loop mode
                       0,                      // flags
                       &_recordQueue);
    printf("ost:%d\n",ost);
}


void CAudioQueuePlayer::UnInitPlayQueue()
{
    AudioQueueDispose(_playQueue, YES);
    _playQueue = NULL;
}

void CAudioQueuePlayer::UnInitRecordQueue()
{
    if (_recordQueue != NULL)
    {
        AudioQueueDispose(_recordQueue, YES);
        _recordQueue = NULL;
    }
}



void CAudioQueuePlayer::setUpPlayQueueBuffers()
{
    for (int t = 0; t < NumberOfAudioDataBuffers; ++t)
    {
        AudioQueueAllocateBuffer(
                                 _playQueue,
                                 _playbytesPerBuffer,
                                 &_playQueueBuffers[t]);
    }
}

void CAudioQueuePlayer::primePlayQueueBuffers()
{
    for (int t = 0; t < NumberOfAudioDataBuffers; ++t)
    {
        PlayCallback(( void *)this, _playQueue, _playQueueBuffers[t]);
    }
}


void CAudioQueuePlayer::setUpRecordQueueBuffers()
{
    for (int t = 0; t < NumberOfAudioDataBuffers; ++t)
    {
        AudioQueueAllocateBuffer(
                                 _recordQueue,
                                 _recordbytesPerBuffer,
                                 &_recordQueueBuffers[t]);
    }
}
void CAudioQueuePlayer::primeRecordQueueBuffers()
{
    for (int t = 0; t < NumberOfAudioDataBuffers; ++t)
    {
        AudioQueueEnqueueBuffer(_recordQueue, _recordQueueBuffers[t], 0, NULL);
    }
}