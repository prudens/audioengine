#ifndef REAL_AUDIO_DEVICE_INTERFACE
#define REAL_AUDIO_DEVICE_INTERFACE
#pragma once
#include<stdint.h>

#ifdef REAL_AUDIO_EXPORTS
#define REAL_AUDIO_API __declspec(dllexport)

#else
#define REAL_AUDIO_EXPORTS __declspec(dllimport)

#endif


#define REAL_AUDIO_CALL __stdcall

extern "C"
{
    /*采集端回调函数指针*/
    typedef void( REAL_AUDIO_CALL*LPRECORDINGDATACALLBACK )( const void* pcm16_data, int len_of_byte );
    typedef intptr_t DID;

    /*创建/销毁一个语音设备*/
    REAL_AUDIO_API DID REAL_AUDIO_CALL CreateDevice();
    REAL_AUDIO_API void REAL_AUDIO_CALL DestroyDevice( DID device_id );


    /*设置/获取采集和播放的采样率以及声道*/
    REAL_AUDIO_API void REAL_AUDIO_CALL SetSampleRate( DID device_id, int32_t rec_sample_rate, int16_t rec_channel, int32_t ply_sample_rate, int16_t ply_channel );
    REAL_AUDIO_API void REAL_AUDIO_CALL GetSampleRate( DID device_id, int32_t* rec_sample_rate, int16_t* rec_channel, int32_t* ply_sample_rate, int16_t* ply_channel );

    /*设置采集端音频数据的接收函数*/
    REAL_AUDIO_API void REAL_AUDIO_CALL SetRecordingDataCallback( DID device_id, LPRECORDINGDATACALLBACK  cb );
    /*播放段音频数据填充函数*/
    REAL_AUDIO_API void REAL_AUDIO_CALL FillPlayoutData( DID device_id, const void*pcm16_data, int len_of_byte );

    /*开始/停止录音*/
    REAL_AUDIO_API bool REAL_AUDIO_CALL StartRecording( DID device_id );
    REAL_AUDIO_API void REAL_AUDIO_CALL StopRecording( DID device_id );

    /*开始/停止播放*/
    REAL_AUDIO_API bool REAL_AUDIO_CALL StartPlayout( DID device_id );
    REAL_AUDIO_API void REAL_AUDIO_CALL StopPlayout( DID device_id );


    REAL_AUDIO_API void REAL_AUDIO_CALL SetFrameSize( DID device_id, int32_t len_of_byte );
    REAL_AUDIO_API int32_t REAL_AUDIO_CALL GetFrameSize( DID device_id );
}



#endif