#include "real_audio_device_interface.h"
#include <stdlib.h>
#include <vector>
intptr_t id = 0;
std::vector<char> vec;
void REAL_AUDIO_CALL RecordingData( const void* pcm16_data, int len_of_byte )
{
    // 发送到远端，这里我就直接放回播放器播放。
    vec.insert( vec.end(), (char*)pcm16_data, (char*)pcm16_data+len_of_byte );
    if (vec.size()>=16000)
    {
        FillPlayoutData( id, vec.data(), len_of_byte );
        vec.erase( vec.begin(), vec.begin() + len_of_byte );
    }
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
    system( "pause" );
    return 0;
}
