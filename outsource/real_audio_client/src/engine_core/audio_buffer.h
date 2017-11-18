#pragma once
#include <memory>
#include <cstdint>
#include "base/time_cvt.hpp"
namespace audio_engine{
#define kMaxAudioFrameSize 1920
	struct AudioBuffer
	{
		int64_t  id;
		tick_t  ts;
		char    data[kMaxAudioFrameSize];
		size_t  length;
		int     samplerate;
		int16_t channel;
		int     rms;
		bool    silent;
	};
	typedef std::shared_ptr<AudioBuffer> AudioBufferPtr;
}
