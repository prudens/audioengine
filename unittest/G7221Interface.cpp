#include "G7221Interface.h"
#include "io/include/audioencoder.h"
#include "device/include/audio_device.h"
#include <stdio.h>
#include "header.h"
#include "io/include/audiodecoder.h"
class G7221EncoderProc : public  AudioBufferProc
{
    AudioEncoder* encoder;
    AudioDecoder* decoder;
    char buf[2048];
    int maxLen = 2048;
    Resampler m_recResample;
    Resampler m_plyResample;
    int m_channel;
    int16_t input[160];
    LPENCODEDATACALLBACK _callback;
    int16_t output[640];
    int outLen = 0;
    int16_t decBuf[640];
    int decLen = 640*2;
    std::list <std::pair<char*,int> > m_plyBufList;
    int16_t pcmData[320*2];
public:
    G7221EncoderProc( const char* file, int samplerate, int channel )
    {
        m_channel = channel;

        encoder = AudioEncoder::Create( AFT_G7221,16000,1,2000 );
        decoder = AudioDecoder::Create( AFT_G7221 );
        if (samplerate == 44100)
        {
            samplerate = 44000;
        }
        m_recResample.Reset( samplerate, 16000, 1 );
        m_plyResample.Reset( 16000, samplerate, 1 );
    }
    G7221EncoderProc( LPENCODEDATACALLBACK callback, int samplerate, int channel )
    {
        _callback = callback;
        m_channel = channel;
        encoder = AudioEncoder::Create( AFT_G7221,16000, 1, 2000 );
        decoder = AudioDecoder::Create( AFT_G7221 );
        if ( samplerate == 44100 )
        {
            samplerate = 44000;
        }
        m_recResample.Reset( samplerate, 16000, 1 );
        m_plyResample.Reset( 16000, samplerate, 1 );
    }
    ~G7221EncoderProc()
    {
        encoder->Release();
        decoder->Release();
    }
    virtual void RecordingDataIsAvailable( const void*data, size_t size_in_byte )
    {
        int16_t*pData = (int16_t*)data;
        size_t len = size_in_byte / 2;
        if ( m_channel == 2 )
        {
            len /= 2;
            for ( size_t i = 0; i < len; i++ )
            {
                pData[i] = pData[i * 2];
            }
        }
        if (len == 441)
        {
            len = 440;
        }
        size_t outLen = 0;
        int ret = m_recResample.Push( pData, len, input, 160, outLen );
        maxLen = 2048;
        encoder->Encode( (int16_t*)input, 160 * 2, buf, maxLen );

        if ( maxLen > 0 )
        {
            if ( _callback )
            {
                memcpy( pcmData + 160, input, 320 );
                _callback( buf, maxLen, pcmData, 640);
            }
        }
        else
        {
            memcpy( pcmData, input, 320);
        }

    };
    virtual size_t NeedMorePlayoutData( void*data, size_t size_in_byte )
    {
        int outSize = size_in_byte / 2;
        if (m_channel == 2)
        {
            outSize /= 2;
        }
        if (outSize == 441)
        {
            outSize = 440;
        }
        size_t MaxLen = 0;
        if (outLen == 0 )
        {
            outLen = 320;
            if ( _callback )
            {
                _callback( output, 40,0,0 );
            }
            else
                return 0;
            decoder->Decode( output, 40, decBuf, decLen );
        }
        int ret = m_plyResample.Push( decBuf + 320 - outLen, 160, (int16_t*)data, outSize, MaxLen );
        outLen -= 160;
        int16_t*pData = (int16_t*)data;
        if (outSize == 440)
        {
            pData[440] = pData[439];
        }
        if (m_channel == 2)
        {
            for ( int i = size_in_byte/2-1; i >= 0;i-- )
            {
                pData[i * 2 + 1] = pData[i];
                pData[i * 2] = pData[i];
            }
        }

        return size_in_byte;
    }

    void AddEncodeData( char* encData, int len )
    {
        char* pDecBuf = new char[decLen];
        decoder->Decode( encData, len, (int16_t*)pDecBuf, decLen );

        m_plyBufList.push_back( std::make_pair( pDecBuf ,decLen) );
    }
    void ClearEncodeBufList()
    {

    }
};

void* G7221_CALL CreateEncoder( LPENCODEDATACALLBACK cb )
{
    AudioDevice* pWinDevice = AudioDevice::Create( AudioDevice::eCoreAudio );
    pWinDevice->Initialize();
    pWinDevice->SetRecordingFormat( 16000, 2 );
    uint32_t samplerate;
    uint16_t channel;
    pWinDevice->GetRecordingFormat( samplerate, channel );
    G7221EncoderProc *proc = new G7221EncoderProc( cb, samplerate, channel );
    pWinDevice->SetAudioBufferCallback( proc );
    return pWinDevice;
}

bool G7221_CALL StartEncode( void* encoder )
{
    AudioDevice* pWinDevice = (AudioDevice*)encoder;
    if (!pWinDevice)
    {
        return false;
    }
    return pWinDevice->StartRecording();
}

void G7221_CALL StopEncode( void* encoder )
{
    AudioDevice* pWinDevice = (AudioDevice*)encoder;
    pWinDevice->StopRecording();
}

void G7221_CALL DeleteEncoder( void* encoder )
{
    AudioDevice* pWinDevice = (AudioDevice*)encoder;

    pWinDevice->Terminate();
    delete pWinDevice->GetAudioBufferCallback();
    pWinDevice->SetAudioBufferCallback(nullptr);
    pWinDevice->Release();
}

void* G7221_CALL CreateDecoder( LPENCODEDATACALLBACK  cb )
{
    AudioDevice* pWinDevice = AudioDevice::Create( AudioDevice::eCoreAudio );
    pWinDevice->Initialize();
    pWinDevice->SetRecordingFormat( 16000, 2 );
    uint32_t samplerate;
    uint16_t channel;
    pWinDevice->GetRecordingFormat( samplerate, channel );
    G7221EncoderProc *proc = new G7221EncoderProc( cb, samplerate, channel );
    pWinDevice->SetAudioBufferCallback( proc );
    return pWinDevice;
}

bool G7221_CALL StartDecode( void* decoder )
{
    AudioDevice* pWinDevice = (AudioDevice*)decoder;
    if ( !pWinDevice )
    {
        return false;
    }
    return pWinDevice->StartPlayout();
}

void G7221_CALL StopDecode( void* decoder )
{
    AudioDevice* pWinDevice = (AudioDevice*)decoder;
    if ( !pWinDevice )
    {
        return ;
    }
    pWinDevice->StopPlayout();
}

void G7221_CALL DeleteDecoder( void*decoder )
{
    if (decoder)
    {
        AudioDevice* pWinDevice = (AudioDevice*)decoder;
        pWinDevice->Terminate();
        pWinDevice->Release();
        pWinDevice = nullptr;
    }
}

G7221_API void G7221_CALL Decode( void* decoder, void* encData, int len )
{
    if (decoder)
    {
        AudioDevice* pWinDevice = (AudioDevice*)decoder;
        auto proc = pWinDevice->GetAudioBufferCallback();
        G7221EncoderProc*pDecodeProc = (G7221EncoderProc*)proc;
        pDecodeProc->AddEncodeData( (char*)encData, len );
    }
}

