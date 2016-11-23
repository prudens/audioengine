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
    void RecordingReset(size_t inFreq, size_t inChannel,size_t outFreq,size_t outChannel );
    void PlayoutReset( size_t inFreq, size_t inChannel, size_t outFreq, size_t outChannel );
    void ProcessCaptureStream( int16_t*  audio_samples, size_t frame_byte_size, int16_t*outSample, size_t& len_of_byte );
    void ProcessRenderStream( int16_t*  audio_samples, size_t frame_byte_size, int16_t*outSample, size_t& len_of_byte );//for aec
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
private:
    bool m_bInit;
    AudioProcessing *m_apm;
    bool m_bEnable;
    Resampler m_recResample;
    Resampler m_plyResample;
    Resampler m_recReverseResample;
    Resampler m_plyReverseResample;
    int32_t m_nCheckVad;
    int32_t m_stream_delay;
    int32_t m_nNormalVoice; 
    int m_level;
    bool m_bSilent;
    std::list<int16_t*> m_audiolist;
    rtc::scoped_ptr<webrtc::RealFourier> m_fft;
public:
    struct {
        size_t infreq = 48000;
        size_t outfreq = 16000;
        size_t inchannel = 2;
        size_t outchannel = 2;
        size_t channel = 2;
        size_t frame_size;
    }rec_resample,ply_resample;

};