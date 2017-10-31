#pragma once
#include <cstdint>
#include "io_common_define.h"
namespace audio_engine
{
	class AudioEncoder
	{
	public:
		static AudioEncoder* Create( AudioFileType type, int samplerate, int16_t channel, int bitrate );
		virtual void Release() = 0;
		virtual bool SetBitRate( int32_t bitRate ) = 0;
		virtual bool Encode( int16_t* pcmData, int inLen, char* encodeData, int& outLen ) = 0;
	};
}