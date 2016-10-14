#include "include/audiodecoder.h"
#include "g7221decoder.h"
#include "aacdecoder.h"
#include "opusdecoder.h"
AudioDecoder*AudioDecoder::Create( AudioFileType type, int samplerate, int channel )
{
    AudioDecoder* pEncoder = nullptr;
    switch ( type )
    {
    case AFT_DEFAULT:
        break;
    case AFT_PCM:
        break;
    case AFT_WAV:
        break;
    case AFT_MP3:
        break;
    case AFT_AAC:
        pEncoder = new AACDecoder();
        break;
    case AFT_G7221:
        pEncoder = new G7221Decoder( samplerate, (int16_t)channel );
        break;
    case AFT_OPUS:
        pEncoder = new OPUSDecoder( samplerate, channel );
    default:
        break;
    }

    return pEncoder;
}