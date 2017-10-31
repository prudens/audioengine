#include "device/include/audio_device.h"
#include "windows_core_audio.h"
#include "windows_audio_dsound.h"
#include "windows_audio_wave.h"
namespace audio_engine
{
	AudioDevice* AudioDevice::Create( DeviceAPI api )
	{
		if(api == eCoreAudio)
		{
			return new WindowsCoreAudio();
		}
		else if(api == eDSound)
		{
			return new WindowsAudioDSound();
		}
		else if(api == eWave)
		{
			return new WindowsAudioWave();
		}
		return nullptr;
	}
}