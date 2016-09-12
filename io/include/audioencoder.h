#pragma once
#include <cstdint>
#include "io_common_define.h"
class AudioEncoder
{
public:
    static AudioEncoder* Create( AudioFileType type );
    virtual void Release() = 0;
    virtual bool Encode( int16_t* pcmData, int inLen, void* encodeData, int& outLen ) = 0;
};