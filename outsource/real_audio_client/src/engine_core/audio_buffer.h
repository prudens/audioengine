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
		int16_t data[kMaxAudioFrameSize];
		size_t  nsamples;
		int     samplerate;
		int16_t channel;
		int     rms;
		bool    silent;
	};
	typedef std::shared_ptr<AudioBuffer> AudioBufferPtr;
}
