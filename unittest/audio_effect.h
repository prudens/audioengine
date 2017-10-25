#pragma once
/*!
 * \file audio_effect.h
 * \date 2016/06/01 13:26
 *
 * \author zhangnaigan
 * Contact: zhangng@snailgame.net
 *
 * \brief 语音信号处理器，目前封装了webrtc audio_processing模块，主要提供给android使用，其他平台也可以使用
 *
 * TODO: long description
 *
 * \note
*/

//system file
#include <stdint.h>
#include <list>
//3rd file
#include "webrtc/modules/audio_processing/include/audio_processing.h"
#include "webrtc/common_audio/resampler/include/resampler.h"
#include "webrtc/common_audio/real_fourier.h"
//project file

#include "audio_low_pass_filter.h"

using namespace webrtc;
class AudioEffect
{
public:
    AudioEffect();
    ~AudioEffect();
public:
    enum
    {
        kTargetRecSampleRate = 16000,
        kTargetPlySampleRate = 16000,
        kAecTotalDelayMs     = 150,
    };
    void Init(size_t recSampleRate,size_t recChannel,size_t plySampleRate,size_t plyChannel);
    void ProcessCaptureStream( int16_t*  audio_samples, size_t frame_byte_size );
    void ProcessRenderStream( int16_t*  audio_samples, size_t frame_byte_size );//for aec
    void EnableAec( bool bEnable );
    void EnableAecm( bool bEnable );
    void EnableVad( bool bEnable );
    void EnableHighPassFilter( bool bEnable );
    void EnableAgc( bool bEnable );
    void EnableLowPassFilter( bool bEnable );
    void EnableNs( bool bEnable );
    void EnableAudioEffect( bool bEnable );
    bool HasVoice()const;
    bool HadProcessingVoice();
    int  GetLevel() { return m_level; }
    bool GetRecordingData(void* data, size_t size_in_byte,bool bNoCache);
private:
    //IParseParamNotify
    void ParseParamNotify( const std::string& Param );
    void UpdateLevelList(bool bsilent,int level);
    bool m_bInit;
    AudioProcessing *m_apm;
    bool m_bEnable;
    size_t m_recSampleRate;
    size_t m_recChannel;
    size_t m_plySampleRate;
    size_t m_plyChannel;
    Resampler m_recResample;
    Resampler m_plyResample;
    Resampler m_recReverseResample;
    Resampler m_plyReverseResample;
    int32_t m_nCheckVad;
    int32_t m_stream_delay;
    int32_t m_nNormalVoice; 
    int m_level;
    bool m_bSilent;
    struct LevelInfo
    {
        int level;
        bool silent;
    };
    std::list<LevelInfo> m_levels;
    std::list<int16_t*> m_audiolist;
    LowPassFilter m_lpf;
    rtc::scoped_ptr<webrtc::RealFourier> m_fft;

};