//
//  AudioUnitDevice.hpp
//  audioqueuedemo
//
//  Created by 陳偉榮 on 16/4/7.
//  Copyright © 2016年 snailgame. All rights reserved.
//

#ifndef AudioUnitDevice_hpp
#define AudioUnitDevice_hpp


#include <stdio.h>
#include <stdint.h>
#include <AudioUnit/AudioUnit.h>

#include <memory>
#include <deque>
#include <mutex>
#include <atomic>


#include "audio_session_observer.h"
#include "audio_device.h"
//#include "RTCAudioSessionDelegateAdapter.h"



typedef std::function<int(char* data,int max_len)> AudioUnitBufferCallback;
class AudioDeviceIOS:public AudioSessionObserver, public AudioDevice
{
public:
    AudioDeviceIOS();
    ~AudioDeviceIOS();
    
    
    virtual void Release()override;
    virtual bool Initialize() override ;
    virtual void Terminate() override;
    
    virtual size_t GetRecordingDeviceNum()const override;
    virtual size_t GetPlayoutDeviceNum()const override;
    
    virtual bool GetPlayoutDeviceName(
                                      int16_t index,
                                      wchar_t name[kAdmMaxDeviceNameSize],
                                      wchar_t guid[kAdmMaxGuidSize] ) override;
    virtual bool RecordingDeviceName(
                                     int16_t index,
                                     wchar_t name[kAdmMaxDeviceNameSize],
                                     wchar_t guid[kAdmMaxGuidSize] ) override;
    
    virtual bool SetPlayoutDevice( int16_t index ) override;
    virtual bool SetRecordingDevice( int16_t index ) override;
    
    virtual bool IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels ) override;
    virtual bool IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels ) override;
    virtual bool SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels ) override;
    virtual bool SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels ) override;
    virtual bool GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels ) override;
    virtual bool GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels ) override;
    
    virtual bool InitPlayout() override;
    virtual bool InitRecording() override;
    
    virtual bool StartPlayout() override;
    virtual bool StopPlayout() override;
    virtual bool Playing() const override;
    virtual bool StartRecording() override;
    virtual bool StopRecording() override;
    virtual bool Recording() const override;
    virtual void SetAudioBufferCallback( AudioBufferProc* pCallback ) override;
    virtual bool SetPropertie( AudioPropertyID id, void* ) override;
    virtual bool GetProperty( AudioPropertyID id, void* ) override;

    
    void Pause();
    void Continue();
    
    AudioUnitBufferCallback playcallback;
    AudioUnitBufferCallback recordcallback;
    
    
    virtual void OnInterruptionBegin()override{Pause();}
    
    // Called when audio session interruption ends.
    virtual void OnInterruptionEnd()override{Continue();}
    
    // Called when audio route changes.
    virtual void OnValidRouteChange()override{}
private:
    // Activates our audio session, creates and initializes the voice-processing
    // audio unit and verifies that we got the preferred native audio parameters.
    bool InitPlayOrRecord();
    
    // Closes and deletes the voice-processing I/O unit.
    void ShutdownPlayOrRecord();
    
    // Helper method for destroying the existing audio unit.
    void DisposeAudioUnit();
    
    // Callback function called on a real-time priority I/O thread from the audio
    // unit. This method is used to signal that recorded audio is available.
    static OSStatus RecordedDataIsAvailable(
                                            void* in_ref_con,
                                            AudioUnitRenderActionFlags* io_action_flags,
                                            const AudioTimeStamp* time_stamp,
                                            UInt32 in_bus_number,
                                            UInt32 in_number_frames,
                                            AudioBufferList* io_data);
    OSStatus OnRecordedDataIsAvailable(
                                       AudioUnitRenderActionFlags* io_action_flags,
                                       const AudioTimeStamp* time_stamp,
                                       UInt32 in_bus_number,
                                       UInt32 in_number_frames);
    
    // Callback function called on a real-time priority I/O thread from the audio
    // unit. This method is used to provide audio samples to the audio unit.
    static OSStatus GetPlayoutData(void* in_ref_con,
                                   AudioUnitRenderActionFlags* io_action_flags,
                                   const AudioTimeStamp* time_stamp,
                                   UInt32 in_bus_number,
                                   UInt32 in_number_frames,
                                   AudioBufferList* io_data);
    OSStatus OnGetPlayoutData(AudioUnitRenderActionFlags* io_action_flags,
                              UInt32 in_number_frames,
                              AudioBufferList* io_data);
    void SetupAudioBuffersForActiveAudioSession();
    bool SetupAndInitializeVoiceProcessingAudioUnit();
    bool SetFormat(int samperate,int channel);
private:
    AudioUnit vpio_unit_;
    
    //RTCAudioSessionDelegateAdapter* audio_session_observer_;
    
    // Extra audio buffer to be used by the playout side for rendering audio.
    // The buffer size is given by FineAudioBuffer::RequiredBufferSizeBytes().
    
    // Provides a mechanism for encapsulating one or more buffers of audio data.
    // Only used on the recording side.
    AudioBufferList audio_record_buffer_list_;
    
    // Contains the audio data format specification for a stream of audio.
    AudioStreamBasicDescription application_format_;
    
    int ByteSizePerInputPacket_ = 480*2;
    int ByteSizePerOutputPacket_ = 480*2;

    char m_inputbuf[4096*2];
    int inputpos_;
    char m_outputbuf[4096*2];
    int outputpos_;
    std::atomic<int> m_data_len;
    
//////////////////////////////////////////////////////////////////////////////
    bool m_bInitialized = false;
    bool m_recInitialized = false;
    bool m_plyInitialized = false;
    bool m_recording = false;
    bool m_playing = false;
    
    AudioBufferProc*  m_pBufferProc = nullptr;
    
    size_t m_recSampleRate = 48000;
    size_t m_plySampleRate = 48000;
    uint16_t m_recChannels = 1;
    uint16_t m_plyChannels = 1;
    
    
};
#endif /* AudioUnitDevice_hpp */
