#include "include/audiodecoder.h"
#include "g7221decoder.h"

AudioDecoder*AudioDecoder::Create( AudioFileType type )
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
        break;
    case AFT_G7221:
        pEncoder = new G7221Decoder;
        break;
    default:
        break;
    }

    return pEncoder;
}