#include "include/audioencoder.h"
#include "g7721encoder.h"

AudioEncoder*AudioEncoder::Create( AudioFileType type )
{
    AudioEncoder* pEncoder = nullptr;
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
        pEncoder = new G7221Encoder;
        break;
    default:
        break;
    }

    return pEncoder;
}