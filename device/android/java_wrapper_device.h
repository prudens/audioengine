#pragma once
/*!
 * \file Real_AndroidDevice.h
 * \date 2016/06/01 13:35
 *
 * \author zhangnaigan
 * Contact: zhangng@snailgame.net
 *
 * \brief android端的语音设备的采集和播放，通过调用java api 实现
 *
 * TODO: 后续需要分离开，跟opensl混合使用。
 *
 * \note
*/

#include <stdint.h>
#include <list>
#include <mutex>
#include <jni.h>
#include "com_snail_audio_device.h"
#include "device/include/audio_device.h"

class JavaWrapperDevice : public AudioDevice
{
public:
	JavaWrapperDevice();
	virtual ~JavaWrapperDevice();
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
    void CacheDirectBufferAddress( void* rec_direct_buffer_address,
                                   size_t rec_direct_buffer_capacity_in_bytes,
                                   void* ply_direct_buffer_address,
                                   size_t ply_direct_buffer_capacity_in_bytes );
    void RecordDataIsAvailable( int32_t byte_size );
    void NeedMorePlayData( int32_t byte_size );
    friend void JNICALL Java_com_audioengine_audio_AudioDevice_CacheDirectBufferAddress
    ( JNIEnv *env, jobject, jlong ins, jobject rec_byte_buffer, jobject ply_byte_buffer );
    friend void JNICALL Java_com_audioengine_audio_AudioDevice_NeedMorePlayData
    ( JNIEnv *env, jobject, jlong ins, jint len );
    friend void JNICALL Java_com_audioengine_audio_AudioDevice_RecordDataIsAvailable
    ( JNIEnv *env, jobject, jlong ins, jint byte_size );
protected:
    std::mutex       m_lock_audio;
	JNIEnv*	         m_env = nullptr;
	jobject          m_audio_device_ins = nullptr; 
	std::list<char*> m_list_audio;
    bool             m_bOpen;
    void* m_rec_direct_buffer_address = nullptr;
    size_t m_rec_direct_buffer_capacity_in_bytes = 0;
    void* m_ply_direct_buffer_address = nullptr;
    size_t m_ply_direct_buffer_capacity_in_bytes = 0;


    ///////////////////////////////////////////////////////
    bool m_bInit                  = false;
    bool m_bInitRecording         = false;
    bool m_bInitPlayout           = false; 
    bool m_bPlaying               = false;
    bool m_bRecording             = false;
    AudioBufferProc* m_pBufferProc = nullptr;
};

