#include "webrtc_agc.h"
#include "webrtc\modules\audio_processing\agc\legacy\gain_control.h"
#include "audio_resample.h"
#include <stdio.h>

WebrtcAgc::WebrtcAgc()
{
    m_agc = WebRtcAgc_Create(); //1
}

WebrtcAgc::~WebrtcAgc()
{
    WebRtcAgc_Free( m_agc );
}

bool WebrtcAgc::Init( int samplerate, int channel )
{
    if ( samplerate != 16000 && samplerate != 32000 && samplerate != 48000 )
    {
        return false;
    }
    if ( channel != 1 && channel != 2)
    {
        return false;
    }
    WebRtcAgc_Init( m_agc, 0, 255, kAgcModeAdaptiveDigital, samplerate );
    SetTargetLevel( 9 );
    m_samplerate = samplerate;
    m_channel = channel;
    m_init = true;
    return true;
}

void WebrtcAgc::SetTargetLevel( int level )
{
    WebRtcAgcConfig cfg;
    cfg.compressionGaindB = 0;
    cfg.targetLevelDbfs = level;
    cfg.limiterEnable = false;
    WebRtcAgc_set_config( m_agc, cfg );//3
}

bool WebrtcAgc::Process( int16_t* inBuf, int nSample )
{
    if (!m_init)
    {
        return false;
    }

    if (m_channel == 2)
    {
        AudioResample::ToMono( inBuf, nSample );
        nSample /= 2;
    }


    uint8_t saturation_warning = 0;
    int err = WebRtcAgc_VirtualMic( m_agc, &inBuf, 1, nSample, 0, &m_micLevelOut );
    if ( err != 0)
    {
        goto _exit;
    }

    err = WebRtcAgc_Process( m_agc, &inBuf, 1, nSample, &inBuf, m_micLevelOut, &m_micLevelOut, 0, &saturation_warning );
    printf( "%d ", m_micLevelOut );
_exit:
    if (m_channel == 2)
    {
        AudioResample::Tostereo( inBuf, nSample );
        nSample *= 2;
    }
    return err == 0;
}
