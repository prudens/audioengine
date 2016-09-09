#pragma once
#include <list>

#include "../include/audio_mixer.h"
class AudioMixerImpl :public AudioMixer
{
    struct MixerParticipantInfo
    {
        MixerParticipant*    participant = nullptr;     // 语音数据源
        webrtc::AudioFrame*  audioFrame = nullptr;       // 当前语音帧
        bool                 isMixed = false;            // 之前是否参与混音
        bool                 isSilent = true;            // 是否静音
        int32_t              needMixCount = 0;           // 尾音参与混音剩余帧数，这里并不立即停止混音，而是渐渐淡出
        float                weightFactor = 1.0f;        // 混音系数
        uint32_t             energy = 0;

    };
    using MixerParticipantList = std::list < MixerParticipantInfo>;
public:
    AudioMixerImpl( int samplerate, int channel );
    ~AudioMixerImpl();
    virtual void Release() override; // 释放一个混音实例
    virtual void AddParticipant( MixerParticipant *participant )override; // 添加一个参与混音的流
    virtual void RemoveParticipant( MixerParticipant *participant )override;// 移除参与混音的流
    virtual bool GetMixerAudio( webrtc::AudioFrame* audioFrame ) override; // 不断地来获取数据
    void LimitParticipantCount( int32_t count )override;
protected:
    webrtc::AudioFrame* PopAudioFrame();
    void PushAudioFrame(webrtc::AudioFrame* pAudioFrame);
    uint32_t CalculateEnergy( const webrtc::AudioFrame* audioFrame );
    void MixFrameList( MixerParticipantList & mixlist, webrtc::AudioFrame* audioFrame);
private:
    MixerParticipantList m_participants;
    int32_t m_limitCount = 3;
    std::list<webrtc::AudioFrame*> m_audioFramePool;
    
};