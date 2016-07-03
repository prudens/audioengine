#pragma once
#include <vector>
#include <cstdint>

#include <ATLComCli.h>
#include <windows.h>

#include <dmo.h>
#include <Mmsystem.h>
#include <MMDeviceApi.h>
#include <mediaobj.h>
#include <wmcodecdsp.h>
#include <audioclient.h>
#include <EndpointVolume.h>
#include <AudioEngineEndPoint.h>
#include <avrt.h> 


#ifndef IF_FAILED_RETURN
#define IF_FAILED_RETURN(hr) do{if(FAILED(hr)) return hr;}while(0)
#endif //IF_FAILED_RETURN

#ifndef IF_FAILED_JUMP
#define IF_FAILED_JUMP(hr, label) do{if(FAILED(hr)) goto label;}while(0)
#endif //IF_FAILED_JUMP

#ifndef IF_FAILED_EXIT
#define IF_FAILED_EXIT(hr) do{if(FAILED(hr)) goto Exit;}while(0)
#endif//IF_FAILED_EXIT

#ifndef RELEASE_HANDLE
#define RELEASE_HANDLE(handle) do{if(handle != 0) {CloseHandle(handle); handle=nullptr;}}while(0)
#endif//RELEASE_HANDLE

#define MAX_STR_LEN 256
typedef struct
{
    wchar_t szDeviceName[MAX_STR_LEN];
    wchar_t szDeviceID[MAX_STR_LEN];
    bool bIsMicArrayDevice;
    bool bDefaultDevice;
} AUDIO_DEVICE_INFO, *PAUDIO_DEVICE_INFO;
typedef std::vector<AUDIO_DEVICE_INFO>  AUDIO_DEVICE_INFO_LIST;

// Utility class which initializes COM in the constructor (STA or MTA),
// and uninitializes COM in the destructor.
class ScopedCOMInitializer {
public:
    // Enum value provided to initialize the thread as an MTA instead of STA.
    enum SelectMTA { kMTA };

    // Constructor for STA initialization.
    ScopedCOMInitializer()
    {
        Initialize( COINIT_APARTMENTTHREADED );
    }

    // Constructor for MTA initialization.
    explicit ScopedCOMInitializer( SelectMTA /*mta*/ )
    {
        Initialize( COINIT_MULTITHREADED );
    }

    ScopedCOMInitializer::~ScopedCOMInitializer()
    {
        if ( SUCCEEDED( hr_ ) )
            CoUninitialize();
    }

    bool succeeded() const { return SUCCEEDED( hr_ ); }

private:
    void Initialize( COINIT init )
    {
        hr_ = CoInitializeEx( NULL, init );
    }

    HRESULT hr_;

    ScopedCOMInitializer( const ScopedCOMInitializer& );
    void operator=( const ScopedCOMInitializer& );
};

// Avrt
void    SetCurrentThreadName( const char* name );
HRESULT DeviceBindTo( EDataFlow eDataFlow, /* eCapture/eRender */
                      int16_t iDevIdx, /* Device Index. -1 - default device. */
                      IAudioClient **ppAudioClient, /* pointer pointer to IAudioClient interface */
                      IAudioEndpointVolume **ppEndpointVolume,
                      WCHAR** ppszEndpointDeviceId ); // Device ID. Need to be freed in caller with CoTaskMemoryFree if it is not NULL;
HRESULT GetDeviceNum( EDataFlow eDataFlow, UINT &uDevCount );
HRESULT EnumDevice( EDataFlow eDataFlow, AUDIO_DEVICE_INFO_LIST& devices );
HRESULT DeviceIsMicArray( wchar_t szDeviceId[], bool &bIsMicArray );
HRESULT EndpointIsMicArray( IMMDevice* pEndpoint, bool & isMicrophoneArray );
HRESULT GetJackSubtypeForEndpoint( IMMDevice* pEndpoint, GUID* pgSubtype );
HRESULT GetInputJack( IMMDevice* pDevice, CComPtr<IPart>& spPart );
HRESULT GetMicArrayGeometry( wchar_t szDeviceId[], KSAUDIO_MIC_ARRAY_GEOMETRY** ppGeometry, ULONG& cbSize );
HRESULT SetBoolProperty( IPropertyStore* ptrPS, REFPROPERTYKEY key, VARIANT_BOOL value );
HRESULT SetVtI4Property( IPropertyStore* ptrPS, REFPROPERTYKEY key, LONG value );
HANDLE  SetThreadPriority( const char* thread_name );
void    RevertThreadPriority( HANDLE hMmTask );