#include <iostream>
#include <list>
#include <mutex>
#include "../device/include/audio_device.h"
#pragma comment(lib,"../build/winx/Debug/audio_device.lib")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "msdmo")
#pragma comment(lib, "dmoguids")
#pragma comment(lib, "wmcodecdspuuid")
using namespace std;
typedef lock_guard<mutex> lockguard;
class CAudioBufferProc : public  AudioBufferProc
{
    virtual void RecordingDataIsAvailable( const int16_t*data, size_t samples )
    {
       
        char* pData = new char[samples];
        memcpy( pData, data, samples );
        lockguard lg( m_lock );
        m_list.push_back(pData);
    }

    virtual size_t NeedMorePlayoutData( int16_t* data, size_t samples )
    {
        lockguard lg( m_lock );
        if (m_list.empty())
        {
            memset( data, 0, samples );

        }
        else
        {
            char* p = m_list.front();
            memcpy( data, p, samples );
            m_list.pop_front();
        }
        return samples;
    }
    virtual void ErrorOccurred(AudioError aeCode) {}
private:
    list<char*> m_list;
    mutex   m_lock;
};

int main( int argc, char** argv )
{
    AudioDevice* pWinDevice = AudioDevice::Create();
    pWinDevice->Init();
    pWinDevice->SetRecordingFormat(16000,1);
    pWinDevice->SetPlayoutFormat( 16000, 1 );
    pWinDevice->InitPlayout();
    pWinDevice->InitRecording();

    CAudioBufferProc cb;
    pWinDevice->SetAudioBufferCallback(&cb);
    pWinDevice->StartPlayout();
    pWinDevice->StartRecording();
    
    system( "pause" );
    pWinDevice->StopRecording();
    pWinDevice->StopPlayout();
    pWinDevice->Terminate();
    pWinDevice->Release();
    return 0;
}