#pragma once
#ifdef G7221_EXPORTS
#define G7221_API __declspec(dllexport)

#else
#define G7221_API __declspec(dllimport)

#endif


#define G7221_CALL __stdcall
extern "C"
{
    typedef void( __stdcall*LPENCODEDATACALLBACK )( const void* encodeData, int lenofbytes,void* pcmData,int pcmLen );
    G7221_API void* G7221_CALL CreateEncoder( LPENCODEDATACALLBACK  cb );
    G7221_API bool G7221_CALL StartEncode( void* encoder );
    G7221_API void G7221_CALL StopEncode( void* encoder );
    G7221_API void G7221_CALL DeleteEncoder( void* encoder );
    G7221_API void* G7221_CALL CreateDecoder( LPENCODEDATACALLBACK  cb );
    G7221_API bool G7221_CALL StartDecode( void* decoder );
    G7221_API void G7221_CALL Decode( void* decoder,void* encData, int len);
    G7221_API void G7221_CALL StopDecode( void* decoder );
    G7221_API void G7221_CALL DeleteDecoder( void*decoder );
}