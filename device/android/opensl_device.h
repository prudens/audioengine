#pragma once
/*!
 * \file opensl_device.h
 * \date 2016/06/01 13:45
 *
 * \author zhangnaigan
 * Contact: zhangng@snailgame.net
 *
 * \brief 对OpenSL API的封装 
 *
 * TODO: long description
 *
 * \note 稳定性还可以，但是目前只支持android 2.3的版本，建议只在不能用java API正常采集播放的手机上用
*/
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <functional>
#include <mutex>
#include <list>
#include "device/include/audio_device.h"

typedef std::function<int( void* data, int bytes_size )> playCallback;
typedef std::function<void( const void* data, int bytes_size )> recordCallback;
class OpenSLDevice : public AudioDevice
{
public:
    static const int kNumBuffer = 5;
    OpenSLDevice();
    virtual ~OpenSLDevice();

    virtual void Release() override;
    virtual bool Initialize()override;
    virtual void Terminate()override;
    virtual size_t GetRecordingDeviceNum()const override;
    virtual size_t GetPlayoutDeviceNum()const override;

    virtual bool GetPlayoutDeviceName(
        int16_t index,
        wchar_t name[kAdmMaxDeviceNameSize],
        wchar_t guid[kAdmMaxGuidSize] )override;
    virtual bool RecordingDeviceName(
        int16_t index,
        wchar_t name[kAdmMaxDeviceNameSize],
        wchar_t guid[kAdmMaxGuidSize] )override;

    virtual bool SetPlayoutDevice( int16_t index )override;
    virtual bool SetRecordingDevice( int16_t index )override;

    virtual bool IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels )override;
    virtual bool IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels )override;

    virtual bool SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )override;
    virtual bool SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )override;
    virtual bool GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )override;
    virtual bool GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )override;

    virtual bool InitPlayout()override;
    virtual bool InitRecording()override;

    virtual bool StartPlayout()override;
    virtual bool StopPlayout()override;
    virtual bool Playing() const override;
    virtual bool StartRecording()override;
    virtual bool StopRecording()override;
    virtual bool Recording() const override;

    virtual void SetAudioBufferCallback( AudioBufferProc* pCallback )override;
    virtual bool SetPropertie( AudioPropertyID id, void* )override;
    virtual bool GetProperty( AudioPropertyID id, void* )override;


private:
    playCallback playcallback_;
    recordCallback recordcallback_;
    static void PlayBufferQueueCallback( SLAndroidSimpleBufferQueueItf bq, void *context );
    static void RecordBufferQueueCallback( SLAndroidSimpleBufferQueueItf bq, void *context );
    void FillBufferQueue();
    void EnqueuePlayoutData();
    void EnqueueRecordData();
private:
    bool m_bInitialize;
    bool m_bInitPlayout;
    bool m_bInitRecording;
    bool m_bPlaying;
    bool m_bRecording;
    SLuint32 m_recSampleRate;
    SLuint32 m_plySampleRate;
    SLuint32 m_recChannel;
    SLuint32 m_plyChannel;

    // engine interfaces
    SLObjectItf m_engineObject;
    SLEngineItf m_engineEngine;

    // output mix interfaces
    SLObjectItf m_outputMixObject;

    // buffer queue player interfaces
    SLObjectItf m_plyObject;
    SLPlayItf m_plyPlayer;
    SLAndroidSimpleBufferQueueItf m_plyBufferQueue;

    // recorder interfaces
    SLObjectItf m_recObject;
    SLRecordItf m_recRecorder;
    SLAndroidSimpleBufferQueueItf m_recBufferQueue;

    // size of buffers
    int m_outBufSamples;
    int m_inBufSamples;

    // buffer indexes
    int m_currentInputIndex;
    int currentOutputIndex;

    // current buffer half (0, 1)
    int m_currentOutputBuffer;
    int m_currentInputBuffer;

    // buffers
    SLint16*m_outputBuffer[kNumBuffer];
    SLint16 *m_inputBuffer[kNumBuffer];
    std::mutex m_lock;
    std::list<SLint16*> m_BufferPool;
    std::list<SLint16*> m_BufferList;


    // audio effect
//    SLAndroidEffectItf audio_effect_obj;
    SLAndroidEffectCapabilitiesItf m_effectCapbilitiesobj;

    AudioBufferProc*m_pBufferProc;
};