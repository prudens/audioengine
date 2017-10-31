#pragma once

#include <stdio.h>
#include "io/include/audiowriter.h"
#include "codec/mp3/encoder/include/lame.h"
namespace audio_engine
{
	class OutBuffer
	{
		unsigned char* m_data;
		int m_size;
		int m_used;

	public:

		OutBuffer( size_t size )
		{
			m_size = size;
			m_data = new unsigned char[m_size];
			m_used = 0;
		}

		~OutBuffer()
		{
			delete[] m_data;
		}

		void advance( int i )
		{
			m_used += i;
		}

		int used() const
		{
			return m_used;
		}

		int unused() const
		{
			return m_size - m_used;
		}

		unsigned char* current() { return m_data + m_used; }
		unsigned char* begin() { return m_data; }
	};

	class Mp3FileWriter : public AudioWriter
	{
	public:
		Mp3FileWriter( const char* filename, int samplerate, int channel );
		~Mp3FileWriter();
		virtual void Destroy()override;
		virtual int SampleRate() const override;
		virtual size_t NumChannels() const override;
		virtual size_t NumSamples() const override;
		virtual void WriteSamples( const float* samples, size_t num_samples ) override;
		virtual void WriteSamples( const int16_t* samples, size_t num_samples ) override;
		bool           Initialized() { return m_bInit; }
	private:
		bool m_bInit = false;
		int m_samplerate = 44100;
		int m_channel = 2;
		int m_nSamples = 0;
		FILE* m_mp3file = nullptr;
		lame_t m_lame = nullptr;
		OutBuffer m_OutBuffer;
	};
}