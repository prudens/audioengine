#pragma once
#include <stdio.h>
#include "include/audioreader.h"
#include "include/audiowriter.h"
namespace audio_engine
{
	class PCMFileReader :public AudioReader
	{
	public:
		PCMFileReader( const char* filename );
		virtual void   Destroy() override;
		bool           Initialized() { return m_init; }
		virtual int    SampleRate() const { return 0; }
		virtual size_t NumChannels() const { return 0; }
		virtual size_t NumSamples() const { return m_num_samples; }
		virtual size_t ReadSamples( size_t num_samples, float* samples );
		virtual size_t ReadSamples( size_t num_samples, int16_t* samples );
		virtual size_t RemainSamples()const { return m_remain_samples; }
		virtual bool   SeekSamples( size_t pos );
		virtual bool   SeekTime( double /*sec*/ ) { return false; }
		virtual bool   SetSpeed( double /*times*/ ) { return false; }
	private:
		FILE* m_read_file = nullptr;
		size_t m_remain_samples = 0;
		bool m_init = false;
		size_t m_num_samples = 0;
	};


	class PCMFileWriter :public AudioWriter
	{
	public:
		PCMFileWriter( const char* filename, int samplerate, int channel );
		virtual void   Destroy();
		bool           Initialized() { return m_init; }
		virtual int    SampleRate() const { return m_sample_rate; }
		virtual size_t NumChannels() const { return m_channel; }
		virtual size_t NumSamples() const { return m_num_samples; }
		virtual void   WriteSamples( const float* samples, size_t num_samples );
		virtual void   WriteSamples( const int16_t* samples, size_t num_samples );
	private:
		FILE* m_writer_file = nullptr;
		int m_sample_rate = 0;
		size_t m_channel = 0;
		size_t m_num_samples = 0;
		bool m_init = false;
	};
}