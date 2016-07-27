//
//  AudioQueuePlayer.hpp
//  audio_engine
//
//  Created by zhangng on 16/4/5.
//  Copyright © 2016年 snailgame. All rights reserved.
//

#ifndef AudioQueuePlayer_hpp
#define AudioQueuePlayer_hpp
#include <AudioToolbox/AudioToolbox.h>
#include <stdio.h>
#include <functional>
#define NumberOfAudioDataBuffers 8
typedef std::function<void(AudioQueueBufferRef buffer, AudioStreamBasicDescription audioFormat)> AudioBufferPlayerBlock;
typedef std::function<void(AudioQueueBufferRef buffer)> AudioBufferPlayerBlock2;
typedef std::function<void(AudioQueueBufferRef buffer,AudioStreamBasicDescription audioformat)> AudioBufferRecordBlock;
class CAudioQueuePlayer
{
public:
   // friend void PlayCallback(void *inUserData, AudioQueueRef inAudioQueue, AudioQueueBufferRef inBuffer);
    CAudioQueuePlayer();
    void InitPlayout(int sampleRate, int channels, int bitsPerChannel,int packetsPerBuffer);
    void InitRecord(int sampleRate, int channels, int bitsPerChannel,int packetsPerBuffer);
    void StartPlay();
    void StartRecord();
    void StopPlay();
    void StopRecord();
    void tearDownPlayAudio();
    void tearDownRecordAudio();
    
    AudioBufferPlayerBlock playblock;
    AudioBufferRecordBlock recordblock;
//private:
    void setUpPlayAudio();
    void setUpRecordAudio();
    
    void InitPlayQueue();
    void InitRecordQueue();
    void UnInitPlayQueue();
    void UnInitRecordQueue();

    
    void setUpPlayQueueBuffers();
    void primePlayQueueBuffers();
    void setUpRecordQueueBuffers();
    void primeRecordQueueBuffers();
    
    void setUpAudioSession();
    void tearDownAudioSession();

    bool playing;
    bool recording;

    AudioStreamBasicDescription playaudioFormat;
    AudioStreamBasicDescription recordaudioFormat;
    AudioQueueRef _playQueue;
    AudioQueueRef _recordQueue;
    // the audio queue buffers for the playback audio queue
    AudioQueueBufferRef _playQueueBuffers[NumberOfAudioDataBuffers];
    AudioQueueBufferRef _recordQueueBuffers[NumberOfAudioDataBuffers];
    // the number of audio data packets to use in each audio queue buffer
    int _playpacketsPerBuffer;
    int _recordpacketsPerBuffer;
    // the number of bytes to use in each audio queue buffer
    int _playbytesPerBuffer;
    int _recordbytesPerBuffer;
    float gain;
};
const char* GetIOSDeviceInfo();
#endif /* AudioQueuePlayer_hpp */
