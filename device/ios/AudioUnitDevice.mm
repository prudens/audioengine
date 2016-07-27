//
//  AudioUnitDevice.cpp
//  audioqueuedemo
//
//  Created by 陳偉榮 on 16/4/7.
//  Copyright © 2016年 snailgame. All rights reserved.
//

#include "AudioUnitDevice.h"
#include <iostream>
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
 
#include "audio_session_interruption_notification.h"



using namespace std;
static std::mutex g_lock;
static int g_audio_session_users = 0;

static bool ActivateAudioSession();
static bool VerifyAudioSession(AVAudioSession* session);
static bool DeactivateAudioSession();
void SetupAudioBuffersForActiveAudioSession();
double kPreferredSampleRate = 48000.0;
const double kPreferredIOBufferDuration = 0.01;
int kPreferredNumberOfChannels = 1;
UInt32 kBytesPerSample = 2;

const int kMaxNumberOfAudioUnitInitializeAttempts = 5;
static void InterruptionListenerCallback(void *inUserData, UInt32 interruptionState)
{
     AudioDeviceIOS *player = ( AudioDeviceIOS *)inUserData;
     if (interruptionState == kAudioSessionBeginInterruption)
     {
         player->Pause();
         printf("interruotion by other app \n");
     }
     else if (interruptionState == kAudioSessionEndInterruption)
     {
         player->Continue();
         printf("begin run audio unit again\n");
     }
}



AudioDeviceIOS::AudioDeviceIOS()
{
    outputpos_ = 0;
    inputpos_ = 0;
    m_data_len.store(0);
}

AudioDeviceIOS::~AudioDeviceIOS()
{
    Terminate();
    
}

void AudioDeviceIOS::Release()
{
    delete this;
}
bool AudioDeviceIOS::Initialize()
{
    AudioComponentDescription vpio_unit_description;
    vpio_unit_description.componentType = kAudioUnitType_Output;
    vpio_unit_description.componentSubType = kAudioUnitSubType_VoiceProcessingIO;
    vpio_unit_description.componentManufacturer = kAudioUnitManufacturer_Apple;
    vpio_unit_description.componentFlags = 0;
    vpio_unit_description.componentFlagsMask = 0;
    
    // Obtain an audio unit instance given the description.
    AudioComponent found_vpio_unit_ref =
    AudioComponentFindNext(nullptr, &vpio_unit_description);
    
    // Create a Voice-Processing IO audio unit.
    OSStatus result = noErr;
    result = AudioComponentInstanceNew(found_vpio_unit_ref, &vpio_unit_);
    if (result != noErr) {
        vpio_unit_ = nullptr;
        return false;
    }
    
    // A VP I/O unit's bus 1 connects to input hardware (microphone). Enable
    // input on the input scope of the input element.
    AudioUnitElement input_bus = 1;
    UInt32 enable_input = 1;
    result = AudioUnitSetProperty(vpio_unit_, kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Input, input_bus, &enable_input,
                                  sizeof(enable_input));
    if (result != noErr) {
        DisposeAudioUnit();
        return false;
    }
    
    // A VP I/O unit's bus 0 connects to output hardware (speaker). Enable
    // output on the output scope of the output element.
    AudioUnitElement output_bus = 0;
    UInt32 enable_output = 1;
    result = AudioUnitSetProperty(vpio_unit_, kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Output, output_bus,
                                  &enable_output, sizeof(enable_output));
    if (result != noErr) {
        DisposeAudioUnit();
        return false;
    }
    m_bInitialized = true;
    return true;
}

void AudioDeviceIOS::Terminate()
{
    if (!m_bInitialized) {
        return;
    }
    StopPlayout();
    StopRecording();
    m_bInitialized = false;
}

size_t AudioDeviceIOS::GetRecordingDeviceNum()const
{
    return 1;
}

size_t AudioDeviceIOS::GetPlayoutDeviceNum()const
{
    return 1;
}


 bool AudioDeviceIOS::GetPlayoutDeviceName(
                                  int16_t index,
                                  wchar_t name[kAdmMaxDeviceNameSize],
                                  wchar_t guid[kAdmMaxGuidSize] )
{
    return false;
}
bool AudioDeviceIOS::RecordingDeviceName(
                                 int16_t index,
                                 wchar_t name[kAdmMaxDeviceNameSize],
                                 wchar_t guid[kAdmMaxGuidSize] )
{
    return false;
}

bool AudioDeviceIOS::SetPlayoutDevice( int16_t index )
{
    return false;
}

bool AudioDeviceIOS::SetRecordingDevice(int16_t index)
{
    return false;
}

bool AudioDeviceIOS::IsRecordingFormatSupported(uint32_t nSampleRate, uint16_t nChannels)
{
    if (!m_bInitialized) {
        return false;
    }
    return SetFormat(nSampleRate, nChannels);
}


bool AudioDeviceIOS::IsPlayoutFormatSupported(uint32_t nSampleRate, uint16_t nChannels)
{
    if (!m_bInitialized) {
        return false;
    }
    return SetFormat(nSampleRate, nChannels);
}


bool AudioDeviceIOS::SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    if (!m_bInitialized) {
        return false;
    }
    m_recSampleRate = nSampleRate;
    m_recChannels = nChannels;
    return SetFormat(nSampleRate, nChannels);
}

bool AudioDeviceIOS::SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels )
{
    if (!m_bInitialized) {
        return false;
    }
    m_plySampleRate = nSampleRate;
    m_plyChannels = nChannels;
    return SetFormat(nSampleRate, nChannels);
}

bool AudioDeviceIOS::GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    if (!m_bInitialized) {
        return false;
    }
    if (m_recSampleRate == 0 || m_recChannels < 1) {
        return false;
    }
    nSampleRate = (uint32_t)m_recSampleRate;
    nChannels = m_recChannels;
    return true;
}

bool AudioDeviceIOS::GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels )
{
    if (!m_bInitialized) {
        return false;
    }
    if (m_plySampleRate == 0 || m_plyChannels < 1) {
        return false;
    }
    nSampleRate = (uint32_t)m_plySampleRate;
    nChannels = m_plyChannels;
    return true;
}

bool AudioDeviceIOS::InitPlayout()
{
    if (!m_bInitialized) {
        return  false;
    }
    if (m_plyInitialized) {
        return true;
    }
    
    if (!m_recInitialized) {
        if(!InitPlayOrRecord())
        {
            return false;
        }
    }
    
    m_plyInitialized = true;
    return true;
}

bool AudioDeviceIOS::InitRecording()
{
    if (!m_bInitialized) {
        return false;
    }
    if (m_recInitialized) {
        return true;
    }
    if (!m_plyInitialized) {
        if(!InitPlayOrRecord())
        {
            return false;
        }
    }
    audio_record_buffer_list_.mNumberBuffers = 1;
    AudioBuffer* audio_buffer = &audio_record_buffer_list_.mBuffers[0];
    audio_buffer->mNumberChannels = m_recChannels;
    audio_buffer->mDataByteSize = 2048 * audio_buffer->mNumberChannels;
    audio_buffer->mData = new char[audio_buffer->mDataByteSize];
    m_recInitialized = true;
    return true;
}

bool AudioDeviceIOS::StartPlayout()
{
    if (!m_recording) {
        OSStatus result = AudioOutputUnitStart(vpio_unit_);
        if (result != noErr) {
            std::cout << "AudioOutputUnitStart failed for StartPlayout: " << result;
            return false;
        }
        std::cout << "Voice-Processing I/O audio unit is now started";
    }
    m_playing = true;
    return true;
}

bool AudioDeviceIOS::StopPlayout()
{
    if (!m_plyInitialized || !m_playing) {
        return true;
    }
    if (!m_recording) {
        ShutdownPlayOrRecord();
    }
    m_plyInitialized = false;
    m_playing=false;
    return true;
}

bool AudioDeviceIOS::Playing() const
{
    return m_playing;
}

bool AudioDeviceIOS::StartRecording()
{
    if (!m_playing) {
        OSStatus result = AudioOutputUnitStart(vpio_unit_);
        if (result != noErr) {
            std::cout << "AudioOutputUnitStart failed for StartRecording: " << result;
            return false;
        }
        std::cout << "Voice-Processing I/O audio unit is now started";
    }
    
    m_recording = true;
    return true;
}

bool AudioDeviceIOS::StopRecording()
{
    if (!m_recInitialized || !m_recording) {
        return true;
    }
    if (!m_playing) {
        ShutdownPlayOrRecord();
    }
    m_recInitialized = false;
    m_recording = false;
    
    AudioBuffer* audio_buffer = &audio_record_buffer_list_.mBuffers[0];
    delete (char*)audio_buffer->mData;
    audio_buffer->mData = NULL;
    
    return true;
}

bool AudioDeviceIOS::Recording() const
{
    return m_recording;
}

void AudioDeviceIOS::SetAudioBufferCallback( AudioBufferProc* pCallback )
{
    m_pBufferProc = pCallback;
}

bool AudioDeviceIOS::SetPropertie( AudioPropertyID id, void* )
{
    return false;
}

bool AudioDeviceIOS::GetProperty( AudioPropertyID id, void* )
{
    return  false;
}


bool AudioDeviceIOS::SetFormat(int samperate,int channel)
{
    // Set the application formats for input and output:
    // - use same format in both directions
    // - avoid resampling in the I/O unit by using the hardware sample rate
    // - linear PCM => noncompressed audio data format with one frame per packet
    // - no need to specify interleaving since only mono is supported
    AudioStreamBasicDescription application_format = {0};
    UInt32 size = sizeof(application_format);
    application_format.mSampleRate = m_recSampleRate;
    application_format.mFormatID = kAudioFormatLinearPCM;
    application_format.mFormatFlags =
    kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    application_format.mBytesPerPacket = kBytesPerSample;
    application_format.mFramesPerPacket = 1;  // uncompressed
    application_format.mBytesPerFrame = kBytesPerSample;
    application_format.mChannelsPerFrame = kPreferredNumberOfChannels;
    application_format.mBitsPerChannel = 8 * 2;
    // Store the new format.
    application_format_ = application_format;
    
    AudioUnitElement input_bus = 1;
    // Set the application format on the output scope of the input element/bus.
    OSStatus result = AudioUnitSetProperty(vpio_unit_, kAudioUnitProperty_StreamFormat,
                                           kAudioUnitScope_Output, input_bus,
                                           &application_format, size);
    if (result != noErr) {
        return false;
    }
    
    AudioUnitElement output_bus = 0;
    // Set the application format on the input scope of the output element/bus.
    result = AudioUnitSetProperty(vpio_unit_, kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input, output_bus,
                                  &application_format, size);
    if (result != noErr) {
        return false;
    }
    
    return true;
}




void AudioDeviceIOS::Pause()
{
    if (nullptr != vpio_unit_) {
        auto result = AudioOutputUnitStop(vpio_unit_);
        if (result != noErr) {
            std::cout << "AudioOutputUnitStop failed: " << result;
        }
    }
}


void AudioDeviceIOS::Continue()
{
    if (nullptr != vpio_unit_) {
        auto result = AudioOutputUnitStart(vpio_unit_);
        if (result != noErr) {
            std::cout << "AudioOutputUnitStop failed: " << result;
        }
    }
}


bool AudioDeviceIOS::InitPlayOrRecord()
{
    if (!ActivateAudioSession()) {
        return false;
    }
    
    // Ensure that the active audio session has the correct category and mode.
    AVAudioSession* session = [AVAudioSession sharedInstance];
    if (!VerifyAudioSession(session)) {
        DeactivateAudioSession();
        return false;
    }
    
    auto audio_session_observer = [AudioSessionInterruptionNotification sharedInstance];
    [audio_session_observer setObserver:this];
    // Create, setup and initialize a new Voice-Processing I/O unit.
    if (!SetupAndInitializeVoiceProcessingAudioUnit()) {
        // Reduce usage count for the audio session and possibly deactivate it if
        // this object is the only user.
        DeactivateAudioSession();
        return false;
    }

    return true;
}


void AudioDeviceIOS::ShutdownPlayOrRecord() {
 
    // Close and delete the voice-processing I/O unit.
    OSStatus result = -1;
    if (nullptr != vpio_unit_) {
        result = AudioOutputUnitStop(vpio_unit_);
        if (result != noErr) {
            std::cout << "AudioOutputUnitStop failed: " << result;
        }
        result = AudioUnitUninitialize(vpio_unit_);
        if (result != noErr) {
            std::cout << "AudioUnitUninitialize failed: " << result;
        }
        DisposeAudioUnit();
    }
    
    auto audio_session_observer = [AudioSessionInterruptionNotification sharedInstance];
    [audio_session_observer setObserver:nil];
    
    // All I/O should be stopped or paused prior to deactivating the audio
    // session, hence we deactivate as last action.
    DeactivateAudioSession();
}


void AudioDeviceIOS::DisposeAudioUnit() {
    if (nullptr == vpio_unit_)
        return;
    OSStatus result = AudioComponentInstanceDispose(vpio_unit_);
    if (result != noErr) {
        std::cout << "AudioComponentInstanceDispose failed:" << result;
    }
    vpio_unit_ = nullptr;
}

OSStatus AudioDeviceIOS::RecordedDataIsAvailable(
                                                 void* in_ref_con,
                                                 AudioUnitRenderActionFlags* io_action_flags,
                                                 const AudioTimeStamp* in_time_stamp,
                                                 UInt32 in_bus_number,
                                                 UInt32 in_number_frames,
                                                 AudioBufferList* io_data) {
      AudioDeviceIOS* audio_device_ios = static_cast<AudioDeviceIOS*>(in_ref_con);
    return audio_device_ios->OnRecordedDataIsAvailable(
                                                       io_action_flags, in_time_stamp, in_bus_number, in_number_frames);
}

OSStatus AudioDeviceIOS::OnRecordedDataIsAvailable(
                                                   AudioUnitRenderActionFlags* io_action_flags,
                                                   const AudioTimeStamp* in_time_stamp,
                                                   UInt32 in_bus_number,
                                                   UInt32 in_number_frames) {
    OSStatus result = noErr;
    // Simply return if recording is not enabled.
    if (!m_recording)
        return result;
    //std::lock_guard<std::mutex> ls(m_inputlock);
    //return noErr;
    // Obtain the recorded audio samples by initiating a rendering cycle.
    // Since it happens on the input bus, the |io_data| parameter is a reference
    // to the preallocated audio buffer list that the audio unit renders into.
    // TODO(henrika): should error handling be improved?

    //printf("m_count:%d\n",m_count.load());
    AudioBufferList* io_data = &audio_record_buffer_list_;
    result = AudioUnitRender(vpio_unit_, io_action_flags, in_time_stamp,
                             in_bus_number, in_number_frames, io_data);
    if (result != noErr) {
        return result;
    }
    // Get a pointer to the recorded audio and send it to the WebRTC ADB.
    // Use the FineAudioBuffer instance to convert between native buffer size
    // and the 10ms buffer size used by WebRTC.
    const UInt32 data_size_in_bytes = in_number_frames*2*m_recChannels;
    
    char* data = static_cast<char*>(io_data->mBuffers[0].mData);
    
    //PushPlaySound(data, data_size_in_bytes);
    //return noErr;
    memcpy(m_inputbuf+inputpos_, data, data_size_in_bytes );
    inputpos_ += data_size_in_bytes;
    while (inputpos_ > ByteSizePerInputPacket_) {
        //PushPlaySound(m_inputbuf, ByteSizePerInputPacket_);
        if(recordcallback)recordcallback(m_inputbuf, ByteSizePerInputPacket_);
        std::copy(m_inputbuf+ByteSizePerInputPacket_,m_inputbuf+inputpos_,m_inputbuf);
        inputpos_ -= ByteSizePerInputPacket_;
        m_data_len += ByteSizePerInputPacket_;
    }
    
    return noErr;
}

OSStatus AudioDeviceIOS::GetPlayoutData(
                                        void* in_ref_con,
                                        AudioUnitRenderActionFlags* io_action_flags,
                                        const AudioTimeStamp* in_time_stamp,
                                        UInt32 in_bus_number,
                                        UInt32 in_number_frames,
                                        AudioBufferList* io_data) {
    AudioDeviceIOS* audio_device_ios = static_cast<AudioDeviceIOS*>(in_ref_con);
    return audio_device_ios->OnGetPlayoutData(io_action_flags, in_number_frames,
                                              io_data);
}

OSStatus AudioDeviceIOS::OnGetPlayoutData(
                                          AudioUnitRenderActionFlags* io_action_flags,
                                          UInt32 in_number_frames,
                                          AudioBufferList* io_data) {

    // Get pointer to internal audio buffer to which new audio data shall be
    // written.
    const UInt32 dataSizeInBytes = io_data->mBuffers[0].mDataByteSize;

    SInt8* destination = static_cast<SInt8*>(io_data->mBuffers[0].mData);
    // Produce silence and give audio unit a hint about it if playout is not
    // activated.
    if (!m_playing)
    {
        *io_action_flags |= kAudioUnitRenderAction_OutputIsSilence;
        memset(destination, 0, dataSizeInBytes);
        return noErr;
    }

    //GetPlaySound(destination,dataSizeInBytes);
    //return noErr;
    
    // 从上层取数据
    while (outputpos_< dataSizeInBytes) {
        //int write_len = GetPlaySound(m_outputbuf+outputpos_,ByteSizePerOutputPacket_);
        int write_len = 0;
       if(playcallback)
           write_len = playcallback(m_outputbuf+outputpos_,ByteSizePerOutputPacket_);
        
        if(write_len == 0)
            break;
        outputpos_ += write_len;
        m_data_len -= write_len;
    }

    if (outputpos_>= dataSizeInBytes) {
        memcpy(destination, m_outputbuf, dataSizeInBytes);
        std::copy(m_outputbuf+dataSizeInBytes,m_outputbuf+outputpos_,m_outputbuf);
        outputpos_ -= dataSizeInBytes;
    }else {
        if (outputpos_ == 0) {
            *io_action_flags |= kAudioUnitRenderAction_OutputIsSilence;
        }
        memcpy(destination, m_outputbuf, outputpos_);
        memset(m_outputbuf+outputpos_, 0, dataSizeInBytes - outputpos_);
        outputpos_ = 0;
    }
    return noErr;
}


void AudioDeviceIOS::SetupAudioBuffersForActiveAudioSession()
{
    // Verify the current values once the audio session has been activated.
    
    audio_record_buffer_list_.mNumberBuffers = 1;
    AudioBuffer* audio_buffer = &audio_record_buffer_list_.mBuffers[0];
    audio_buffer->mNumberChannels = m_plyChannels;
    audio_buffer->mDataByteSize = 2048 * audio_buffer->mNumberChannels;
    audio_buffer->mData = new char[audio_buffer->mDataByteSize];
}


bool AudioDeviceIOS::SetupAndInitializeVoiceProcessingAudioUnit() {

    // Specify the callback function that provides audio samples to the audio
    // unit.
    
    SetFormat(m_recSampleRate,m_recChannels);
    AudioUnitElement output_bus = 0;
    AudioUnitElement input_bus = 0;
    OSStatus result;
    AURenderCallbackStruct render_callback;
     render_callback.inputProc = GetPlayoutData;
     render_callback.inputProcRefCon = this;
     result = AudioUnitSetProperty(
     vpio_unit_, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
     output_bus, &render_callback, sizeof(render_callback));
     if (result != noErr) {
     DisposeAudioUnit();
     return false;
     }
    
    // Disable AU buffer allocation for the recorder, we allocate our own.
    // TODO(henrika): not sure that it actually saves resource to make this call.
    UInt32 flag = 0;
    result = AudioUnitSetProperty(
                                  vpio_unit_, kAudioUnitProperty_ShouldAllocateBuffer,
                                  kAudioUnitScope_Output, input_bus, &flag, sizeof(flag));
    if (result != noErr) {
        DisposeAudioUnit();
    }
    
    // Specify the callback to be called by the I/O thread to us when input audio
    // is available. The recorded samples can then be obtained by calling the
    // AudioUnitRender() method.
    AURenderCallbackStruct input_callback;
    input_callback.inputProc = RecordedDataIsAvailable;
    input_callback.inputProcRefCon = this;
    result = AudioUnitSetProperty(vpio_unit_,
                                  kAudioOutputUnitProperty_SetInputCallback,
                                  kAudioUnitScope_Global, input_bus,
                                  &input_callback, sizeof(input_callback));
    if (result != noErr) {
        DisposeAudioUnit();
    }
    
    // Initialize the Voice-Processing I/O unit instance.
    // Calls to AudioUnitInitialize() can fail if called back-to-back on
    // different ADM instances. The error message in this case is -66635 which is
    // undocumented. Tests have shown that calling AudioUnitInitialize a second
    // time, after a short sleep, avoids this issue.
    // See webrtc:5166 for details.
    int failed_initalize_attempts = 0;
    result = AudioUnitInitialize(vpio_unit_);
    while (result != noErr) {

        ++failed_initalize_attempts;
        if (failed_initalize_attempts == kMaxNumberOfAudioUnitInitializeAttempts) {
            DisposeAudioUnit();
            return false;
        }
        [NSThread sleepForTimeInterval:0.1f];
        result = AudioUnitInitialize(vpio_unit_);
    }
    printf("AudioUnitInitialize Success!!!\n");
    return true;
}



// Verifies that the current audio session supports input audio and that the
// required category and mode are enabled.
static bool VerifyAudioSession(AVAudioSession* session) {
    
    return true;
    // Ensure that the device currently supports audio input.
    if (!session.isInputAvailable) {
       
        return false;
    }
    
    // Ensure that the required category and mode are actually activated.
    if (![session.category isEqualToString:AVAudioSessionCategoryPlayAndRecord]) {
        
        return false;
    }
    if (![session.mode isEqualToString:AVAudioSessionModeVoiceChat]) {
        return false;
    }
    return true;
}

// Activates an audio session suitable for full duplex VoIP sessions when
// |activate| is true. Also sets the preferred sample rate and IO buffer
// duration. Deactivates an active audio session if |activate| is set to false.
static bool ActivateAudioSession(AVAudioSession* session, bool activate)
{
    @autoreleasepool {
        NSError* error = nil;
        BOOL success = NO;
        
        if (!activate) {
            // Deactivate the audio session using an extra option and then return.
            // AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation is used to
            // ensure that other audio sessions that were interrupted by our session
            // can return to their active state. It is recommended for VoIP apps to
            // use this option.
            success = [session
                       setActive:NO
                       withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation
                       error:&error];
            return success;
        }
        
        // Go ahead and active our own audio session since |activate| is true.
        // Use a category which supports simultaneous recording and playback.
        // By default, using this category implies that our app’s audio is
        // nonmixable, hence activating the session will interrupt any other
        // audio sessions which are also nonmixable.
        if (session.category != AVAudioSessionCategoryPlayAndRecord) {
            error = nil;
            success = [session setCategory:AVAudioSessionCategoryPlayAndRecord
                               withOptions:AVAudioSessionCategoryOptionAllowBluetooth
                                     error:&error];
            
        }
        
        
        /*
        // Specify mode for two-way voice communication (e.g. VoIP).
        if (session.mode != AVAudioSessionModeVoiceChat) {
            error = nil;
            success = [session setMode:AVAudioSessionModeVoiceChat error:&error];
            
        }
        */
        // Set the session's sample rate or the hardware sample rate.
        // It is essential that we use the same sample rate as stream format
        // to ensure that the I/O unit does not have to do sample rate conversion.
        error = nil;
        success = [session setPreferredSampleRate:kPreferredSampleRate error:&error];
        
        
        // Set the preferred audio I/O buffer duration, in seconds.
        error = nil;
        success = [session setPreferredIOBufferDuration:kPreferredIOBufferDuration
                                                  error:&error];
        
        
        // Activate the audio session. Activation can fail if another active audio
        // session (e.g. phone call) has higher priority than ours.
        error = nil;
        success = [session setActive:YES error:&error];
        if (!success) {
            return false;
        }
        
        // Ensure that the active audio session has the correct category and mode.
        if (!VerifyAudioSession(session)) {
            std::cout << "Failed to verify audio session category and mode";
            return false;
        }
        
        // Try to set the preferred number of hardware audio channels. These calls
        // must be done after setting the audio session’s category and mode and
        // activating the session.
        // We try to use mono in both directions to save resources and format
        // conversions in the audio unit. Some devices does only support stereo;
        // e.g. wired headset on iPhone 6.
        // TODO(henrika): add support for stereo if needed.
        error = nil;
        success = [session setPreferredInputNumberOfChannels:kPreferredNumberOfChannels
                                             error:&error];
        error = nil;
        success =[session setPreferredOutputNumberOfChannels:kPreferredNumberOfChannels
                                              error:&error];
        
 
        return true;
    }
}

static bool ActivateAudioSession() {
    std::lock_guard<std::mutex> ls(g_lock);
    if (g_audio_session_users == 0) {
        // The system provides an audio session object upon launch of an
        // application. However, we must initialize the session in order to
        // handle interruptions. Implicit initialization occurs when obtaining
        // a reference to the AVAudioSession object.
        AVAudioSession* session = [AVAudioSession sharedInstance];
        // Try to activate the audio session and ask for a set of preferred audio
        // parameters.
        if (!ActivateAudioSession(session, true)) {
           
            return false;
        }
        
    }
    ++g_audio_session_users;
    return true;
}


static bool DeactivateAudioSession() {

    std::lock_guard<std::mutex> ls(g_lock);
    if (g_audio_session_users == 1) {
        AVAudioSession* session = [AVAudioSession sharedInstance];
        if (!ActivateAudioSession(session, false)) {
            return false;
        }
    }
    --g_audio_session_users;
    
    return true;
}



