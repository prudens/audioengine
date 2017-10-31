#include "io/mp3filereader.h"
#include "io/aacfilereader.h"
#include "io/wav_file.h"
#include "io/pcm_file.h"
namespace audio_engine
{
	AudioReader* AudioReader::Create( const char*filename, AudioFileType type )
	{
		switch(type)
		{
		case AFT_DEFAULT:
			break;
		case AFT_PCM:
		{
			auto pReader = new PCMFileReader( filename );
			if(!pReader->Initialized())
			{
				pReader->Destroy();
				pReader = nullptr;
			}
			return pReader;

		}
		case AFT_WAV:
		{
			auto pReader = new WavReader( filename );
			if(!pReader->Initialized())
			{
				pReader->Destroy();
				pReader = nullptr;
			}
			return pReader;
		}
		case AFT_MP3:
		{
			auto pReader = new Mp3FileReader( filename );
			if(!pReader->Initialized())
			{
				pReader->Destroy();
				return nullptr;
			}
			return pReader;
		}
		break;
		case AFT_AAC:
		{
			auto pReader = new AACFileReader( filename );
			if(!pReader->Initialized())
			{
				pReader->Destroy();
				return nullptr;
			}
			return pReader;
		}
		case AFT_G7221:
			break;
		default:
			break;
		}

		return nullptr;
	}
}