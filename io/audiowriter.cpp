#include "io/include/audiowriter.h"
#include "aacfilewriter.h"
#include "mp3filewriter.h"
#include "wav_file.h"
#include "pcm_file.h"
namespace audio_engine
{
	AudioWriter*AudioWriter::Create( const char* filename, int sample_rate, size_t channels, AudioFileType type )
	{
		switch(type)
		{
		case AFT_AAC:
		{
			auto pWriter = new AACFileWriter( filename, sample_rate, channels );
			if(!pWriter->Initialized())
			{
				pWriter->Destroy();
				return nullptr;
			}
			return pWriter;
		}
		case AFT_MP3:
		{
			auto pWriter = new Mp3FileWriter( filename, sample_rate, channels );
			if(!pWriter->Initialized())
			{
				pWriter->Destroy();
				return nullptr;
			}
			return pWriter;
		}
		case AFT_WAV:
		{
			auto pWriter = new WavWriter( filename, sample_rate, channels );
			if(!pWriter->Initialized())
			{
				pWriter->Destroy();
				pWriter = nullptr;
			}
			return pWriter;
		}
		case AFT_PCM:
		{
			auto pWriter = new PCMFileWriter( filename, sample_rate, channels );
			if(!pWriter->Initialized())
			{
				pWriter->Destroy();
				pWriter = nullptr;
			}
			return pWriter;
		}
		}
		return nullptr;
	}
}