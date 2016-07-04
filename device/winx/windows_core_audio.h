#pragma once
#include <mutex>
#include <atlbase.h>
#include <ATLComCli.h>
#include <uuids.h>

#include "media_buffer.h"
#include "device/include/audio_device.h"
#include "windows_audio_device_helper.h"
#include <bitset>

class WindowsCoreAudio final:public AudioDevice
{
    typedef std::lock_guard<std::mutex> lock_guard;
public:
    WindowsCoreAudio();
    virtual ~WindowsCoreAudio();
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
    virtual bool SetPropertie( AudioPropertyID id, void* );
    virtual bool GetProperty( AudioPropertyID id, void* );
private:
    bool    IsFormatSupported( CComPtr<IAudioClient> audioClient, DWORD nSampleRate, WORD nChannels );
    bool    SetDMOProperties();
    bool    InitRecordingMedia();
    bool    InitRecordingDMO();

    DWORD   DoRenderThread();
    DWORD   DoCaptureThreadPollDMO();
    DWORD   DoCaptureThread();

    static DWORD WINAPI WSAPIRenderThread( LPVOID context );
    static DWORD WINAPI WSAPICaptureThreadPollDMO( LPVOID context );
    static DWORD WINAPI WSAPICaptureThread( LPVOID context );
private:
    WindowsCoreAudio( const WindowsCoreAudio& ) = delete;
    WindowsCoreAudio( const WindowsCoreAudio&& ) = delete;
    WindowsCoreAudio& operator=( WindowsCoreAudio& ) = delete;
    WindowsCoreAudio& operator=( WindowsCoreAudio&& ) = delete;
private:
    ScopedCOMInitializer                    m_comInit;
    std::mutex                              m_audiolock;
    CComPtr<IMediaObject>                   m_spDmo;// DirectX Media Object (DMO) for the built-in AEC.
    CComPtr<IAudioClient>                   m_spClientIn;
    CComPtr<IAudioClient>                   m_spClientOut;
    CComPtr<IAudioRenderClient>             m_spRenderClient;
    CComPtr<IAudioCaptureClient>            m_spCaptureClient;
    CComPtr<IMediaBuffer>                   m_spMediaBuffer;

    AudioBufferProc*                        m_pBufferProc = nullptr;

    size_t                                  m_recSampleRate = 0;
    size_t                                  m_plySampleRate = 0;
    uint16_t                                m_recChannels = 0;
    uint16_t                                m_plyChannels = 0;
    int16_t                                 m_recDevIndex = 0;
    int16_t                                 m_plyDevIndex = 0;

    bool                                    m_bUseDMO = false;
    bool                                    m_DMOIsAvailble = false;
    bool                                    m_bInitialize = false;
    bool                                    m_recIsInitialized = false;
    bool                                    m_playIsInitialized = false;
    bool                                    m_recording = false;
    bool                                    m_playing = false;


    HANDLE                                  m_hRenderSamplesReadyEvent = nullptr;
    HANDLE                                  m_hPlayThread = nullptr;
    HANDLE                                  m_hRenderStartedEvent = nullptr;
    HANDLE                                  m_hShutdownRenderEvent = nullptr;

    HANDLE                                  m_hCaptureSamplesReadyEvent = nullptr;
    HANDLE                                  m_hRecThread = nullptr;
    HANDLE                                  m_hCaptureStartedEvent = nullptr;
    HANDLE                                  m_hShutdownCaptureEvent = nullptr;

    AUDIO_DEVICE_INFO_LIST                  m_CaptureDevices;
    AUDIO_DEVICE_INFO_LIST                  m_RenderDevices;

    std::bitset<sizeof( uint32_t )>           m_setEffect;
};