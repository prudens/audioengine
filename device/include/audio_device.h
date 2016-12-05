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
    AE_FEATRURE_UNSUPPORTED,
    AE_RECORD_FORMAT_CHANGE, // you can call relative function to get new format 
};

enum AudioPropertyID
{
    ID_ENABLE_AEC,                  // enable build in aec
    ID_ENBALE_AGC,                  // enable build in agc
    ID_ENBALE_VAD,                  // enable build in vad
    ID_ENBALE_NS,                   // enable build in ns
    ID_VOLUME,                      // modfiy audio volume
};


class AudioBufferProc
{
public:
    virtual ~AudioBufferProc() {};
    virtual void RecordingDataIsAvailable(const void*data, size_t size_in_byte) = 0;
    virtual size_t NeedMorePlayoutData( void*data, size_t size_in_byte ) = 0;
    virtual void ErrorOccurred( AudioError /*aeCode*/ ) {};
};


class AudioDevice
{
public:
    enum DeviceAPI
    {
        eCoreAudio,
        eDSound,
        eWave,
    };
    static AudioDevice* Create(DeviceAPI api);
    virtual void Release()=0;
    virtual bool Initialize() = 0;
    virtual void Terminate() = 0;

    virtual size_t GetRecordingDeviceNum()const = 0;
    virtual size_t GetPlayoutDeviceNum()const = 0;

    virtual bool GetPlayoutDeviceName(
        int16_t index,
        wchar_t name[kAdmMaxDeviceNameSize],
        wchar_t guid[kAdmMaxGuidSize] ) = 0;
    virtual bool GetRecordingDeviceName(
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

    virtual bool StartPlayout() = 0;
    virtual bool StopPlayout() = 0;
    virtual bool Playing() const = 0;
    virtual bool StartRecording() = 0;
    virtual bool StopRecording() = 0;
    virtual bool Recording() const = 0;
    virtual void SetAudioBufferCallback( AudioBufferProc* pCallback ) = 0;
    virtual AudioBufferProc* GetAudioBufferCallback()const = 0;
    virtual bool SetPropertie( AudioPropertyID id, void* ) = 0;
    virtual bool GetProperty( AudioPropertyID id, void* ) = 0;
protected:
    ~AudioDevice() {}
};