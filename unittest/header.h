#include <iostream>
#include <list>
#include <mutex>
#include <algorithm>
#include <cassert>
#include <complex>
#include <thread>
#include <conio.h>
#include <algorithm>

#include "base/fft.h"
#include "base/audio_util.h"
#include "device/include/audio_device.h"
#include "effect/3d/include/mit_hrtf_lib.h"
#include "io/wav_file.h"
#include "base/circular_buffer.hpp"
#include "effect/3d/include/mixer3d.h"
#include "audio_effect.h"
#include "base/time_cvt.hpp"
#include "base/min_max_heap.hpp"
#include <numeric>
#include "audio_noise_suppression.h"
#include "webrtc/common_audio/signal_processing/include/spl_inl.h"
#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#include "io/include/audioreader.h"
#include "io/include/audiowriter.h"
using  Complex = std::complex < float >;
using namespace std;
typedef lock_guard<mutex> lockguard;