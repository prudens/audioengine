#include "real_audio_device_interface.h"
#include <stdlib.h>
intptr_t id = 0;
void REAL_AUDIO_CALL RecordingData( const void* pcm16_data, int len_of_byte )
{
    // 发送到远端，这里我就直接放回播放器播放。
    FillPlayoutData( id, pcm16_data, len_of_byte );
}

int main()
{
    const size_t time = 40;//ms
    const int32_t sample_rate = 16000;
    const size_t frame_size = sample_rate / 1000 * 2 * time;
    const int32_t channel = 1;
    id = CreateDevice();
    SetSampleRate( id, sample_rate, channel, sample_rate, channel );
    SetFrameSize( id, frame_size );
    SetRecordingDataCallback( id, RecordingData );
    StartPlayout( id );
    StartRecording( id );
    system( "pause" );
    StopPlayout( id );
    StopRecording( id );
    DestroyDevice(id);
    return 0;
}
