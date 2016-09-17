#pragma once
#include <cstdint>
#include "webrtc/modules/include/module_common_types.h"
class MixerParticipant
{
public:
    virtual bool GetAudioFrame( webrtc::AudioFrame* audioFrame ) = 0;
};

class AudioMixer
{
public:
    static AudioMixer* Create(int samplerate,int channel,int frame_size); // 创建一个混音实例
    virtual void Release() = 0; // 释放一个混音实例
    virtual void AddParticipant( MixerParticipant *participant ) = 0; // 添加一个参与混音的流
    virtual void RemoveParticipant( MixerParticipant *participant ) = 0;// 移除参与混音的流
    virtual void LimitParticipantCount(int32_t count) = 0;              // 限制混音方数量
    virtual bool GetMixerAudio( webrtc::AudioFrame* audioFrame ) = 0; // 不断地来获取数据
};