#pragma once
#include <cstdint>
#include "io_common_define.h"
class AudioDecoder
{
public:
    static AudioDecoder* Create( AudioFileType type );
    virtual void Release() = 0;
    virtual bool Decode( void* encodeData, int outLen, int16_t* pcmData, int& inLen ) = 0;
};