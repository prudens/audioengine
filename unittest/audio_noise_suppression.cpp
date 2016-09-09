#include "audio_noise_suppression.h"
#include <string.h>
#include <algorithm>
//#include "macrodef.h"
#include "fft.h"
#include "webrtc/common_audio/include/audio_util.h"
#include "kwindows.h"
#include "fft.h"
#include <functional>
struct bandinfo
{
    float v;
    int idx;
    int start;
    int end;
};

const int BAND_NUM = 16;
static int Band[BAND_NUM + 1] = { 80, 500, 1000, 1500,2000,2500,3000,3500,4000,4500,5000,5500,6000,6500,7000,7500,8000 };


AudioNoiseSuppression::AudioNoiseSuppression()
{

}

AudioNoiseSuppression::~AudioNoiseSuppression()
{
    delete m_energy;
    delete m_noise_buf;
    delete m_proc_buf;
    for ( auto v: audio_cache_free )
    {
        delete v;
    }
}
void AudioNoiseSuppression::Init( int samplerate,int nChannel )
{
    m_recSamplerate = samplerate;
    if (samplerate == 16000)
    {
        m_frame = 256;
        m_pWindows = kBlocks160w256;
        m_fft = webrtc::RealFourier::Create( 8 );
    }
    else if ( samplerate == 48000 )
    {
        m_frame = 512;
        m_pWindows = kBlock480w512;
        m_fft = webrtc::RealFourier::Create(9);

    }
    m_recChannel = nChannel;
    m_energy = new float[m_frame];
    m_noise_buf = new int16_t[m_frame];
    m_proc_buf = new float[m_frame];
    m_pData = new Complex[m_frame];

}
int AudioNoiseSuppression::Analyze( int16_t*speech_frame, int size, bool isSilent )
{
    if (m_recChannel == 2)
    {
        size /= 2;
    }
    memcpy( m_noise_buf, m_noise_buf + size, 2 * ( m_frame - size ) );
    if ( m_recChannel == 1 )
    {
        memcpy( m_noise_buf + m_frame - size, speech_frame, 2 * size );
    }
    else
    {
        for ( int i = 0; i < size; i++ )
        {
            m_noise_buf[m_frame - size + i] = speech_frame[i * 2];
        }
    }

    if ( isSilent )
    {
        return Silent;
    }

    float bandunit = m_frame /  (float)m_recSamplerate;

    float std[BAND_NUM] = { 0 };
    bandinfo bi[BAND_NUM];
    bandinfo cbi[BAND_NUM];
    float bands[BAND_NUM];

    for ( int i = 0; i < m_frame; i++ )
    {
        m_proc_buf[i] = webrtc::S16ToFloat( m_noise_buf[i] ) * m_pWindows[i];
    }

    //CFFT::Forward( m_pData, m_frame );
    m_fft->Forward( m_proc_buf, m_pData );
    m_pData[0] = 0;
    for ( int i = 0; i < BAND_NUM; i++ )
    {
        bands[i] = 0;
        int start = Band[i] * bandunit;
        int end = Band[i + 1] * bandunit;
        for ( int j = start; j < end; j++ )
        {
            m_energy[j] = ( m_pData[j].real() * m_pData[j].real() + m_pData[j].imag() * m_pData[j].imag() );
            bands[i] += m_energy[j];
        }
        bands[i] /= ( end - start );
    }

    float sum = 0;

    for ( int i = 0; i < BAND_NUM; i++ )
    {
        bi[i].idx = i;
        bi[i].v = bands[i];
        bi[i].start = Band[i];
        bi[i].end = Band[i + 1];
        sum += bi[i].v;
    }
    std::sort( bi, bi + BAND_NUM, [] ( const bandinfo& l, const bandinfo& r ) { return l.v > r.v; } );

    int index = 0;
    // 合并
    for ( int i = 0; i < BAND_NUM; i++ )
    {
        if ( i == BAND_NUM - 1 )
        {
            cbi[index].v = bi[i].v;
            cbi[index].start = bi[i].start;
            cbi[index].end = bi[i].end;
            index++;
            break;
        }

        int sub = bi[i].idx - bi[i + 1].idx;
        if ( sub == -1 )
        {
            cbi[index].v = ( bi[i].v + bi[i + 1].v ) / 2;
            cbi[index].start = bi[i].start;
            cbi[index].end = bi[i + 1].end;
            i++;

        }
        else  if ( sub == 1 )
        {
            cbi[index].v = ( bi[i].v + bi[i + 1].v ) / 2;
            cbi[index].start = bi[i + 1].start;
            cbi[index].end = bi[i].end;
            i++;
        }
        else
        {
            cbi[index].v = bi[i].v;
            cbi[index].start = bi[i].start;
            cbi[index].end = bi[i].end;
        }
        index++;
    }
    sum = 0;
    for ( int i = 0; i < index; i++ )
    {
        sum += bi[i].v;
    }

    float speech_proc = bands[0] / sum;
    float scale = cbi[0].v / sum;


    int start = cbi[0].start * bandunit;
    int end = cbi[0].end * bandunit;
    int len = end - start + 1;
    len /= 5;
    for ( int i = 0; i < 5; i++ )
    {
        std[i] = 0;
        for ( int j = 0; j < len; j++ )
        {
            std[i] += m_energy[start + i * len + j];
        }
    }

    sort( std, std + 5, std::greater<float>() );
    float s = 0;
    for ( int i = 0; i < 5; i++ )
    {
        s += std[i];
    }
    float focus = ( std[0] ) / s;
    int type;
    if ( sum < 1 )
    {
        type = Silent;
    }
    else if ( bi[0].idx == 0 )
    {
        type = Speech;
    }
    else if ( cbi[0].start == 80 && focus < 0.9 )
    {
        type = Speech;
    }
    else if ( 0.8 < scale )
    {
        type = Noise;
    }
    else if ( 0.3 < scale  && focus > 0.6 )
    {
        type = Noise;
    }
    else if ( speech_proc < 0.01 && 0.4 < scale && focus > 0.4 )
    {
        type = FullHighFreq;
    }
    else
    {
        type = Consonant;
    }
    return type;
}

AudioNoiseSuppression::VoiceFrame* AudioNoiseSuppression::Proceess( int16_t* data, int size, int level, int silent, int noise_type )
{
    if (data == nullptr)
    {
        // 清空列表
        if (!audio_cache.empty())
        {
            auto p = audio_cache.front();
            audio_cache.pop_front();
            audio_cache_free.push_back( p );
            return p;
        }
        else
        {
            return nullptr;
        }
    }
    if (noise_type == Noise)
    {
        m_nNoise++;
    }
    m_nAudio++;
    if ( m_nAudio > 1000)
    {
        m_nAudio = 1;
        m_nNoise = 0;
    }

    m_future_list.push_back( noise_type );
    VoiceFrame* pFrame = nullptr;
    if ( !audio_cache_free.empty() )
    {
        pFrame = audio_cache_free.front();
        audio_cache_free.pop_front();
    }
    else
    {
       pFrame = new VoiceFrame();
    }
    memcpy( pFrame->data, data, size * 2 );
    pFrame->size = size;
    pFrame->level = level;
    pFrame->silent = silent;
    audio_cache.push_back( pFrame );
    if ( m_future_list.size() < 10 )
    {
        return nullptr;
    }

    //process begin
    int h_noise = -1;
    auto itbefore = m_history_list.begin();
    auto itafter = m_future_list.begin();
    int i = std::min( m_history_list.size(), m_future_list.size() );
    int before,after;
    int nSilent = 0;
    int nSpeech = 0;
    int nNoise = 0;
    int nConsonat = 0;
    if (i> 5)
    {
        i = 5;
    }
    for ( ; i > 0; i-- )
    {
        before = *itbefore;
        after = *itafter;
        if (before == 4) {
            nSilent++;
        }
        else if(before == 0)
        {
            nSpeech++;
        }
        else if(before == 3)
        {
            nNoise++;
        }
        else if ( before == 1)
        {
            nConsonat++;
        }
        
        if (after == 4) {
            nSilent++;
        }
        else if(after == 0)
        {
            nSpeech++;
        }
        else if(after == 3)
        {
            nNoise++;
        }
        else if(after == 1)
        {
            nConsonat++;
        }
        itbefore++;
        itafter++;
    }

    if ( nSpeech >= 3 )
    {
        h_noise = -1;
    }
    else if ( nNoise > nSpeech )
    {
        h_noise = 1;
    }
    else if ( nSpeech == 0 && nNoise == 0)
    {
        h_noise = m_last_nosie_type;
    }
    
    
    static int silent_time = 0;
    nConsonat = 0;
    for ( auto v: m_future_list)
    {
        if (v == 0 )
        {
            //printf("is speech");
            silent_time = 0;
            h_noise = -1;
        }
        else if (v == 1) {
            nConsonat++;
        }
        
    }
    if (h_noise != -1) {
        for ( auto v : m_history_list )
        {
            if ( v == 0 )
            {
                silent_time = 0;
                h_noise = -1;
                break;
            }
            
        }
    }

    if (nConsonat > 6) {
        h_noise = -1;
    }

    if (nSilent > 7)
    {
        silent_time++;
    }
    else
    {
        silent_time = 0;
    }

    if ( nSpeech == 0  && silent_time > 20 )
    {
        //printf("silent_time=%d",silent_time);
        h_noise = 1;
    }

    int nContinue = 0;
    int flag = 0;
    for (auto v: m_history_check) {

        if (flag == 0) {
            flag = v;
            nContinue++;
        }
        else if (flag != v) {
            break;
        }
        else
        {
            nContinue++;
        }
    }
    auto p = audio_cache.front();
    if ( h_noise > 0)
    {
        if (nContinue >= 2) {
            if (flag == 1) {
                //ALOGII( "is noise\n" );
                memset( p->data, 0, size * 2 );
                p->silent = true;
                p->level = 120;
            }
        }
        m_history_check.push_front(1);
    }
    else
    {
        m_history_check.push_front(-1);
    }
    
    m_last_nosie_type = h_noise;

    if (m_history_check.size()> 10) {
        m_history_check.pop_back();
    }

    //proceess end
    audio_cache.pop_front();
    int cur = m_future_list.front();
    m_future_list.pop_front();
    m_history_list.push_front( cur );
    if ( m_history_list.size() > 10 )
    {
        m_history_list.pop_front();
    }


    audio_cache_free.push_back( p );
    return p;
}

void AudioNoiseSuppression::Reset()
{
    m_future_list.clear();
    m_history_list.clear();
    m_history_check.clear();
    m_last_nosie_type = -1;
    m_nAudio = 1;
    m_nNoise = 0;
}
