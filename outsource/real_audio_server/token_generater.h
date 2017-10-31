#pragma once
#include <cstdint>
#include "real_audio_common.h"
namespace audio_engine{
	class TokenGenerater
	{
	public:
		static int64_t NewToken( std::string userid );
		static bool CheckToken( int64_t token );
	};
}