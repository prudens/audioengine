#include "windows_audio_device_helper.h"
#include "functiondiscoverykeys.h"           // PKEY_Device_FriendlyName
#include <strsafe.h>


#define PARTID_MASK 0x0000ffff //HQH ����һ��

// AVRT function pointers
typedef BOOL( WINAPI *PAvRevertMmThreadCharacteristics )( HANDLE );
typedef HANDLE( WINAPI *PAvSetMmThreadCharacteristicsA )( LPCSTR, LPDWORD );
typedef BOOL( WINAPI *PAvSetMmThreadPriority )( HANDLE, AVRT_PRIORITY );

void SetCurrentThreadName( const char* name )
{
    struct {
        DWORD dwType;
        LPCSTR szName;
        DWORD dwThreadID;
        DWORD dwFlags;
    } threadname_info = { 0x1000, name, static_cast<DWORD>( -1 ), 0 };

    __try
    {
        ::RaiseException( 0x406D1388, 0, sizeof( threadname_info ) / sizeof( DWORD ),
                          reinterpret_cast<ULONG_PTR*>( &threadname_info ) );
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
    }
}


///////////////////////////////////////////////////////////////////////////
//
// Function:
//      DeviceBindTo
//
// Description:
//      Bind device to an IAudioClient interface.
//
//  Parameters:
//      eDataFlow: eRender for render device, eCapture for capture device
//      iDevIdx: Index of device in the enumeration list. If it is -1, use default device 
//      ppVoid: pointer pointer to IAudioClient interface.
//      ppszEndpointDeviceId: Device ID. Caller is responsible for freeing memeory
//              using CoTaskMemoryFree. If can be NULL if called doesn't need this info.
//
// Return:
//      S_OK if successful
//
///////////////////////////////////////////////////////////////////////////////
HRESULT DeviceBindTo(
    EDataFlow eDataFlow,    // eCapture/eRender
    int16_t iDevIdx,        // Device Index. -1 - default device. 
    IAudioClient **ppAudioClient,    // pointer pointer to IAudioClient interface
    IAudioEndpointVolume **ppEndpointVolume,
    WCHAR** ppszEndpointDeviceId )   // Device ID. Need to be freed in caller with CoTaskMemoryFree if it is not NULL
{
    HRESULT hResult;

    CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDeviceCollection> spEndpoints;
    CComPtr<IMMDevice> spDevice;
    WCHAR *pszDeviceId = NULL;

    if ( ppAudioClient == NULL )
        return E_POINTER;

    *ppAudioClient = NULL;

    hResult = spEnumerator.CoCreateInstance( __uuidof( MMDeviceEnumerator ) );
    IF_FAILED_JUMP( hResult, Exit );

    // use default device
    if ( iDevIdx < 0 )
    {
        hResult = spEnumerator->GetDefaultAudioEndpoint( eDataFlow, eCommunications, &spDevice );
        IF_FAILED_JUMP( hResult, Exit );
    }
    else
    {
        // User selected device
        hResult = spEnumerator->EnumAudioEndpoints( eDataFlow, DEVICE_STATE_ACTIVE, &spEndpoints );
        IF_FAILED_JUMP( hResult, Exit );

        hResult = spEndpoints->Item( iDevIdx, &spDevice );
        IF_FAILED_JUMP( hResult, Exit );
    }

    // get device ID and format
    hResult = spDevice->GetId( &pszDeviceId );
    IF_FAILED_JUMP( hResult, Exit );

    // Active device
    hResult = spDevice->Activate( __uuidof( IAudioClient ), CLSCTX_INPROC_SERVER, NULL, (void**)ppAudioClient );
    IF_FAILED_JUMP( hResult, Exit );

    if ( ppEndpointVolume )
    {
        hResult = spDevice->Activate( __uuidof( IAudioEndpointVolume ), CLSCTX_INPROC_SERVER, NULL, (void **)ppEndpointVolume );
        IF_FAILED_JUMP( hResult, Exit );
    }

Exit:
    if ( ppszEndpointDeviceId )
        *ppszEndpointDeviceId = pszDeviceId;
    else if ( pszDeviceId )
        CoTaskMemFree( pszDeviceId );

    return hResult;
}


///////////////////////////////////////////////////////////////////////////////
//
// Function:
//      GetDeviceNum
//
// Description:
//      Enumerate audio device and return the device information.
//
//  Parameters:
//      eDataFlow: eRender for render device, eCapture for capture device
//      uDevCount: Number of device
//
// Return:
//      S_OK if successful
//
///////////////////////////////////////////////////////////////////////////////
HRESULT GetDeviceNum( EDataFlow eDataFlow, UINT &uDevCount )
{
    HRESULT hResult = S_OK;

    CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDeviceCollection> spEndpoints;

    hResult = spEnumerator.CoCreateInstance( __uuidof( MMDeviceEnumerator ) );
    IF_FAILED_RETURN( hResult );

    hResult = spEnumerator->EnumAudioEndpoints( eDataFlow, DEVICE_STATE_ACTIVE, &spEndpoints );
    IF_FAILED_RETURN( hResult );

    hResult = spEndpoints->GetCount( &uDevCount );
    IF_FAILED_RETURN( hResult );

    return hResult;

}


///////////////////////////////////////////////////////////////////////////
//
// Function:
//      EnumDevice
//
// Description:
//      Enumerate audio device and return the device information.
//
//  Parameters:
//      eDataFlow: eRender for render device, eCapture for capture device
//      uNumElements: Size of audio device info structure array.
//      pDevicInfo: device info structure array. Caller is responsible to allocate and free
//                  memory. The array size is specified by uNumElements.
//
// Return:
//      S_OK if successful
//
///////////////////////////////////////////////////////////////////////////////
HRESULT EnumDevice( EDataFlow eDataFlow, AUDIO_DEVICE_INFO_LIST& devices )
{
    devices.clear();
    HRESULT hResult = S_OK;
    WCHAR* pszDeviceId = NULL;
    WCHAR* pszDefaultDeviceID = nullptr;
    PROPVARIANT value;
    UINT index, dwCount;
    bool IsMicArrayDevice;

    CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDeviceCollection> spEndpoints;

    CComPtr<IMMDevice> spDevice;


    hResult = spEnumerator.CoCreateInstance( __uuidof( MMDeviceEnumerator ) );
    IF_FAILED_JUMP( hResult, Exit );

    hResult = spEnumerator->GetDefaultAudioEndpoint( eDataFlow, eCommunications, &spDevice );
    IF_FAILED_JUMP( hResult, Exit );
    spDevice->GetId( &pszDefaultDeviceID );
    IF_FAILED_JUMP( hResult, Exit );
    spDevice.Release();

    hResult = spEnumerator->EnumAudioEndpoints( eDataFlow, DEVICE_STATE_ACTIVE, &spEndpoints );
    IF_FAILED_JUMP( hResult, Exit );

    hResult = spEndpoints->GetCount( &dwCount );
    IF_FAILED_JUMP( hResult, Exit );

    devices.resize( dwCount );

    for ( index = 0; index < dwCount; index++ )
    {
        CComPtr<IMMDevice> spDevice;
        CComPtr<IPropertyStore> spProperties;

        PropVariantInit( &value );

        hResult = spEndpoints->Item( index, &spDevice );
        IF_FAILED_JUMP( hResult, Exit );

        hResult = spDevice->GetId( &pszDeviceId );
        IF_FAILED_JUMP( hResult, Exit );

        hResult = spDevice->OpenPropertyStore( STGM_READ, &spProperties );
        IF_FAILED_JUMP( hResult, Exit );

        hResult = spProperties->GetValue( PKEY_Device_FriendlyName, &value );
        IF_FAILED_JUMP( hResult, Exit );

        EndpointIsMicArray( spDevice, IsMicArrayDevice );

        StringCchCopyW( devices[index].szDeviceID, MAX_STR_LEN - 1, pszDeviceId );
        StringCchCopyW( devices[index].szDeviceName, MAX_STR_LEN - 1, value.pwszVal );
        devices[index].bIsMicArrayDevice = IsMicArrayDevice;
        devices[index].bDefaultDevice = !wcscmp( pszDeviceId, pszDefaultDeviceID );

        PropVariantClear( &value );
        CoTaskMemFree( pszDeviceId );
        pszDeviceId = NULL;


    }

Exit:
    if ( pszDefaultDeviceID )
    {
        CoTaskMemFree( pszDefaultDeviceID );
    }
    return hResult;
}


///////////////////////////////////////////////////////////////////////////////
// Function:
//      DeviceIsMicArray
//
// Description:
//      Determines if a given IMMDevice is a microphone array by device ID
//
// Returns:
//      S_OK on success
///////////////////////////////////////////////////////////////////////////////
HRESULT DeviceIsMicArray( wchar_t szDeviceId[], bool &bIsMicArray )
{
    HRESULT hr = S_OK;

    if ( szDeviceId == NULL )
        return E_INVALIDARG;

    CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDevice> spDevice;

    hr = spEnumerator.CoCreateInstance( __uuidof( MMDeviceEnumerator ) );
    IF_FAILED_RETURN( hr );

    hr = spEnumerator->GetDevice( szDeviceId, &spDevice );
    IF_FAILED_RETURN( hr );

    hr = EndpointIsMicArray( spDevice, bIsMicArray );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
// Function:
//      EndpointIsMicArray
//
// Description:
//      Determines if a given IMMDevice is a microphone array by Endpoint pointer
//
// Returns:
//      S_OK on success
///////////////////////////////////////////////////////////////////////////////
HRESULT EndpointIsMicArray( IMMDevice* pEndpoint, bool & isMicrophoneArray )
{
    if ( pEndpoint == NULL )
        return E_POINTER;

    GUID subType = { 0 };

    HRESULT hr = GetJackSubtypeForEndpoint( pEndpoint, &subType );

    isMicrophoneArray = ( subType == KSNODETYPE_MICROPHONE_ARRAY ) ? true : false;

    return hr;
}// EndpointIsMicArray()


///////////////////////////////////////////////////////////////////////////////
// Function:
//      GetJackSubtypeForEndpoint
//
// Description:
//      Gets the subtype of the jack that the specified endpoint device
//      is plugged into.  e.g. if the endpoint is for an array mic, then
//      we would expect the subtype of the jack to be 
//      KSNODETYPE_MICROPHONE_ARRAY
//
// Return:
//      S_OK if successful
//
///////////////////////////////////////////////////////////////////////////////
HRESULT GetJackSubtypeForEndpoint( IMMDevice* pEndpoint, GUID* pgSubtype )
{
    HRESULT hr = S_OK;

    if ( pEndpoint == NULL )
        return E_POINTER;

    CComPtr<IDeviceTopology>    spEndpointTopology;
    CComPtr<IConnector>         spPlug;
    CComPtr<IConnector>         spJack;
    CComQIPtr<IPart>            spJackAsPart;

    // Get the Device Topology interface
    hr = pEndpoint->Activate( __uuidof( IDeviceTopology ), CLSCTX_INPROC_SERVER,
                              NULL, (void**)&spEndpointTopology );
    IF_FAILED_JUMP( hr, Exit );

    hr = spEndpointTopology->GetConnector( 0, &spPlug );
    IF_FAILED_JUMP( hr, Exit );

    hr = spPlug->GetConnectedTo( &spJack );
    IF_FAILED_JUMP( hr, Exit );

    spJackAsPart = spJack;

    hr = spJackAsPart->GetSubType( pgSubtype );

Exit:
    return hr;
}//GetJackSubtypeForEndpoint()


///////////////////////////////////////////////////////////////////////////////
// GetInputJack() -- Gets the IPart interface for the input jack on the
//                   specified device.
///////////////////////////////////////////////////////////////////////////////
HRESULT GetInputJack( IMMDevice* pDevice, CComPtr<IPart>& spPart )
{
    HRESULT hr = S_OK;

    if ( pDevice == NULL )
        return E_POINTER;

    CComPtr<IDeviceTopology>    spTopology;
    CComPtr<IConnector>         spPlug;
    CComPtr<IConnector>         spJack = NULL;

    // Get the Device Topology interface
    hr = pDevice->Activate( __uuidof( IDeviceTopology ),
                            CLSCTX_INPROC_SERVER, NULL,
                            reinterpret_cast<void**>( &spTopology ) );
    IF_FAILED_RETURN( hr );

    hr = spTopology->GetConnector( 0, &spPlug );
    IF_FAILED_RETURN( hr );

    hr = spPlug->GetConnectedTo( &spJack );
    IF_FAILED_RETURN( hr );

    // QI for the part
    spPart = spJack;
    if ( spPart == NULL )
        return E_NOINTERFACE;

    return hr;
}// GetInputJack()


///////////////////////////////////////////////////////////////////////////////
// Function:
//      GetMicArrayGeometry()
//
// Description:
//      Obtains the geometry for the specified mic array.
//
// Parameters:  szDeviceId -- The requested device ID, which can be obtained
//                            from calling EnumAudioCaptureDevices()
//              
//              ppGeometry -- Address of the pointer to the mic-array gemometry.  
//                            Caller is ressponsible for calling CoTaskMemFree()
//                            if the call is successfull.
//
//              cbSize -- size of the geometry structure
//
// Returns:     S_OK on success
///////////////////////////////////////////////////////////////////////////////
HRESULT GetMicArrayGeometry( wchar_t szDeviceId[], KSAUDIO_MIC_ARRAY_GEOMETRY** ppGeometry, ULONG& cbSize )
{
    HRESULT hr = S_OK;

    if ( szDeviceId == NULL )
        return E_INVALIDARG;
    if ( ppGeometry == NULL )
        return E_POINTER;

    cbSize = 0;
    CComPtr<IMMDeviceEnumerator> spEnumerator;
    CComPtr<IMMDevice>           spDevice;
    CComQIPtr<IPart>             spPart;
    bool bIsMicArray;

    hr = spEnumerator.CoCreateInstance( __uuidof( MMDeviceEnumerator ) );
    IF_FAILED_RETURN( hr );

    hr = spEnumerator->GetDevice( szDeviceId, &spDevice );
    IF_FAILED_RETURN( hr );

    hr = EndpointIsMicArray( spDevice, bIsMicArray );
    IF_FAILED_RETURN( hr );

    if ( !bIsMicArray )
        return E_FAIL;

    UINT nPartId = 0;
    hr = GetInputJack( spDevice, spPart );
    IF_FAILED_RETURN( hr );

    hr = spPart->GetLocalId( &nPartId );
    IF_FAILED_RETURN( hr );

    CComPtr<IDeviceTopology>     spTopology;
    CComPtr<IMMDeviceEnumerator> spEnum;
    CComPtr<IMMDevice>           spJackDevice;
    CComPtr<IKsControl>          spKsControl;
    wchar_t *                    pwstrDevice = 0;

    // Get the topology object for the part
    hr = spPart->GetTopologyObject( &spTopology );
    IF_FAILED_RETURN( hr );

    // Get the id of the IMMDevice that this topology object describes.
    hr = spTopology->GetDeviceId( &pwstrDevice );
    IF_FAILED_RETURN( hr );

    // Get an IMMDevice pointer using the ID
    hr = spEnum.CoCreateInstance( __uuidof( MMDeviceEnumerator ) );
    IF_FAILED_JUMP( hr, Exit );

    hr = spEnum->GetDevice( pwstrDevice, &spJackDevice );
    IF_FAILED_JUMP( hr, Exit );

    // Activate IKsControl on the IMMDevice
    hr = spJackDevice->Activate( __uuidof( IKsControl ), CLSCTX_INPROC_SERVER,
                                 NULL, reinterpret_cast<void**>( &spKsControl ) );
    IF_FAILED_JUMP( hr, Exit );

    // At this point we can use IKsControl just as we would use DeviceIoControl
    KSP_PIN ksp;
    ULONG   cbGeometry = 0;

    // Inititialize the pin property
    ::ZeroMemory( &ksp, sizeof( ksp ) );
    ksp.Property.Set = KSPROPSETID_Audio;
    ksp.Property.Id = KSPROPERTY_AUDIO_MIC_ARRAY_GEOMETRY;
    ksp.Property.Flags = KSPROPERTY_TYPE_GET;
    ksp.PinId = nPartId & PARTID_MASK;

    // Get data size by passing NULL
    hr = spKsControl->KsProperty( reinterpret_cast<PKSPROPERTY>( &ksp ),
                                  sizeof( ksp ), NULL, 0, &cbGeometry );
    IF_FAILED_JUMP( hr, Exit );

    // Allocate memory for the microphone array geometry
    *ppGeometry = reinterpret_cast<KSAUDIO_MIC_ARRAY_GEOMETRY*>
        ( ::CoTaskMemAlloc( cbGeometry ) );

    if ( *ppGeometry == 0 )
    {
        hr = E_OUTOFMEMORY;
    }
    IF_FAILED_JUMP( hr, Exit );

    // Now retriev the mic-array structure...
    DWORD cbOut = 0;
    hr = spKsControl->KsProperty( reinterpret_cast<PKSPROPERTY>( &ksp ),
                                  sizeof( ksp ), *ppGeometry, cbGeometry,
                                  &cbOut );
    IF_FAILED_JUMP( hr, Exit );
    cbSize = cbGeometry;

Exit:
    if ( pwstrDevice != 0 )
    {
        ::CoTaskMemFree( pwstrDevice );
    }
    return hr;
}//GetMicArrayGeometry()


HRESULT SetBoolProperty( IPropertyStore* ptrPS,
                                           REFPROPERTYKEY key,
                                           VARIANT_BOOL value )
{
    PROPVARIANT pv;
    PropVariantInit( &pv );
    pv.vt = VT_BOOL;
    pv.boolVal = value;
    HRESULT hr = ptrPS->SetValue( key, pv );
    PropVariantClear( &pv );
    return hr;
}

HRESULT SetVtI4Property( IPropertyStore* ptrPS,
                                           REFPROPERTYKEY key,
                                           LONG value )
{
    PROPVARIANT pv;
    PropVariantInit( &pv );
    pv.vt = VT_I4;
    pv.lVal = value;
    HRESULT hr = ptrPS->SetValue( key, pv );
    PropVariantClear( &pv );
    return hr;
}


HANDLE SetThreadPriority( const char* thread_name )
{
    SetCurrentThreadName( thread_name /*"windows_core_audio_capture_thread"*/ );

    HANDLE hMmTask = NULL;

    // Try to load the Avrt DLL
    // Get handle to the Avrt DLL module.
    HMODULE avrtLibrary = LoadLibrary( TEXT( "Avrt.dll" ) );
    if ( avrtLibrary )
    {
        // Handle is valid (should only happen if OS larger than vista & win7).
        // Try to get the function addresses.

        auto AvSetMmThreadCharacteristicsA = (PAvSetMmThreadCharacteristicsA)GetProcAddress( avrtLibrary, "AvSetMmThreadCharacteristicsA" );
        auto AvSetMmThreadPriority = (PAvSetMmThreadPriority)GetProcAddress( avrtLibrary, "AvSetMmThreadPriority" );
        if ( AvSetMmThreadCharacteristicsA && AvSetMmThreadPriority )
        {
            DWORD taskIndex( 0 );
            hMmTask = AvSetMmThreadCharacteristicsA( "Pro Audio", &taskIndex );
            if ( hMmTask )
            {
                if ( !AvSetMmThreadPriority( hMmTask, AVRT_PRIORITY_CRITICAL ) )
                {
                    printf( "failed to boost rec-thread using MMCSS" );
                }
                printf( "%s is now registered with MMCSS (taskIndex=%d)",
                        thread_name, taskIndex );
            }
            else
            {
                printf( "failed to enable MMCSS on capture thread (err=%d)",
                        GetLastError() );
            }
        }
        FreeLibrary( avrtLibrary );
    }
    return hMmTask;
}


void RevertThreadPriority( HANDLE hMmTask )
{
    if ( hMmTask == NULL )
    {
        return;
    }
    // Get handle to the Avrt DLL module.
    HMODULE  avrtLibrary = ::LoadLibrary( TEXT( "Avrt.dll" ) );
    if ( avrtLibrary )
    {
        // Handle is valid (should only happen if OS larger than vista & win7).
        // Try to get the function addresses.
        auto AvRevertMmThreadCharacteristics = (PAvRevertMmThreadCharacteristics)GetProcAddress( avrtLibrary, "AvRevertMmThreadCharacteristics" );
        if ( AvRevertMmThreadCharacteristics )
        {
            AvRevertMmThreadCharacteristics( hMmTask );
        }
        ::FreeLibrary( avrtLibrary );
    }
}