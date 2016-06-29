#pragma once
#include <cstdint>

static const int kAdmMaxDeviceNameSize = 128;
static const int kAdmMaxFileNameSize = 512;
static const int kAdmMaxGuidSize = 128;

static const int kAdmMinPlayoutBufferSizeMs = 10;
static const int kAdmMaxPlayoutBufferSizeMs = 250;

enum AudioError
{
    AE_OK,
    AE_UNSUPPORTED,
};

class AudioBufferProc
{
public:
    virtual ~AudioBufferProc() {};
    virtual void RecordingDataIsAvailable(const void*data, size_t size_in_byte) = 0;
    virtual size_t NeedMorePlayoutData( void*data, size_t size_in_byte ) = 0;
    virtual void ErrorOccurred(AudioError aeCode) = 0;
};


class AudioDevice
{
public:
    static AudioDevice* Create();
    void Release() { delete this; }
    virtual bool Initialize() = 0;
    virtual void Terminate() = 0;

    virtual size_t GetRecordingDeviceNum()const = 0;
    virtual size_t GetPlayoutDeviceNum()const = 0;

    virtual bool GetPlayoutDeviceName(
        int16_t index,
        wchar_t name[kAdmMaxDeviceNameSize],
        wchar_t guid[kAdmMaxGuidSize] ) = 0;
    virtual bool RecordingDeviceName(
        int16_t index,
        wchar_t name[kAdmMaxDeviceNameSize],
        wchar_t guid[kAdmMaxGuidSize] ) = 0;

    virtual bool SetPlayoutDevice( int16_t index ) = 0;
    virtual bool SetRecordingDevice( int16_t index ) = 0;

    virtual bool IsRecordingFormatSupported( uint32_t nSampleRate, uint16_t nChannels ) = 0;
    virtual bool IsPlayoutFormatSupported( uint32_t nSampleRate, uint16_t nChannels ) = 0;
    virtual bool SetRecordingFormat( uint32_t nSampleRate, uint16_t nChannels ) = 0;
    virtual bool SetPlayoutFormat( uint32_t nSampleRate, uint16_t nChannels ) = 0;
    virtual bool GetRecordingFormat( uint32_t& nSampleRate, uint16_t& nChannels ) = 0;
    virtual bool GetPlayoutFormat( uint32_t& nSampleRate, uint16_t& nChannels ) = 0;

    virtual bool InitPlayout() = 0;
    virtual bool InitRecording() = 0;

    virtual bool StartPlayout() = 0;
    virtual bool StopPlayout() = 0;
    virtual bool Playing() const = 0;
    virtual bool StartRecording() = 0;
    virtual bool StopRecording() = 0;
    virtual bool Recording() const = 0;
    virtual void SetAudioBufferCallback( AudioBufferProc* pCallback ) = 0;
protected:
    virtual ~AudioDevice() {}
};