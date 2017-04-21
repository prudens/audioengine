#pragma once
#include <cstdint>
class WebrtcAgc
{
public:
    WebrtcAgc();
    ~WebrtcAgc();
    bool Init(int samplerate, int channel);
    void SetTargetLevel(int level);//0-31
    bool Process( int16_t* inBuf, int nSample);
private:
    void* m_agc = nullptr;
    bool m_init = false;
    int m_samplerate = 16000;
    int m_channel = 1;
    int32_t m_micLevelOut = 0;
};