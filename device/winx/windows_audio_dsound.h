#include "device/include/audio_device.h"
#include "windows_audio_device_helper.h"
#include <mutex>
class WindowsAudioDSound : public AudioDevice
{
    struct DEVICE_INFO
    {
        GUID guid;
        char name[256];
    };
    typedef std::vector<DEVICE_INFO> DEVICE_INFO_LIST;
    enum {
        MA_DELAY = 120,
    };
public:
    WindowsAudioDSound();
    virtual ~WindowsAudioDSound();
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

    virtual void SetAudioBufferCallback( AudioBufferProc* pCallback ) override;
    virtual AudioBufferProc* GetAudioBufferCallback()const override;

    virtual bool SetPropertie( AudioPropertyID id, void* );
    virtual bool GetProperty( AudioPropertyID id, void* );
private:
    static BOOL CALLBACK EnumCaptrueCallback( GUID* pGUID, LPCSTR szDesc, LPCSTR szDrvName, void* pContext );
     DWORD  DoRenderThread( );
    static DWORD WINAPI WSAPICaptureThread( LPVOID context );
private:
    AUDIO_DEVICE_INFO_LIST capture_devices_;
    AUDIO_DEVICE_INFO_LIST render_devices_;
    bool initialize_ = false;
    std::mutex capture_lock_;
    std::mutex render_lock_;

    int capture_device_index_;
    int render_device_index_;

    int capture_sample_rate_;
    int render_sample_rate_;
    int capture_channel_;
    int render_channel_;

    LPDIRECTSOUND capture_direct_sound_ = nullptr;
    LPDIRECTSOUND render_direct_sound_ = nullptr;
    LPDIRECTSOUNDBUFFER  render_direct_sound_buf_ = nullptr;
    bool init_playout_ = false;
    bool init_recording_ = false;
    bool playing_ = false;
    bool recording_ = false;

    HANDLE     playout_thread_handle_;
    HANDLE     recording_thread_handle_;
    HANDLE     wait_playout_thread_start_handle_;
    HANDLE     wait_recording_thread_start_handle_;

};
