#include "audio_resample.h"
#include <algorithm>

AudioResample::AudioResample()
{

}

bool AudioResample::Reset( int32_t inSampleRate, int16_t inChannel, int32_t outSamplerate, int16_t outChannel )
{
    m_inSamplerate = inSampleRate;
    m_inChannel = inChannel;
    m_outSamplerate = outSamplerate;
    m_outChannel = outChannel;
    if (m_inSamplerate == 44100)
    {
        inSampleRate = 44000;
    }
    if (outSamplerate == 44100)
    {
        outSamplerate = 44000;
    }
    size_t channel = std::min(inChannel, outChannel);
    m_ResampleImpl.Reset( m_inSamplerate, outSamplerate, channel );
    return false;
}

bool AudioResample::Process( int16_t* inBuf, size_t inSamples, int16_t* outBuf, size_t outSamples )
{
    if (m_inChannel == 2 && m_outChannel == 2)
    {

    }
    if ( m_inSamplerate  == 44100)
    {
        
    }
    return false;
}
