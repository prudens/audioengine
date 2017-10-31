#pragma once
#include "include/audioencoder.h"
#include "codec/aac/libAACenc/include/aacenc_lib.h"
#include "base/circular_buffer.hpp"
namespace audio_engine
{
	class AACEncoder :public AudioEncoder
	{
	public:
		AACEncoder( int samplerate, int16_t channel, int bitrate );
		~AACEncoder();
		void Release()override;
		bool SetBitRate( int32_t bitRate )override;
		bool Encode( int16_t* pcmData, int inLen, char* encodeData, int& outLen )override;
		void Clear( char* encodeData, int& outLen );
	private:
		int32_t _bitrate;

		HANDLE_AACENCODER m_hAacEncoder = nullptr;
		bool m_bInit = false;
		int m_samplerate = 44100;
		int m_channel = 2;
		int m_nSamples = 0;
		FILE* m_aacfile = nullptr;

		int m_framesize;

		AACENC_BufDesc m_encinBuf;
		int m_in_eisize = 2;
		int m_in_bufsize;
		int m_in_buf_id = IN_AUDIO_DATA;

		AACENC_BufDesc m_encoutBuf;
		int m_out_eisize = 2;
		int m_out_bufsize;
		int m_out_buf_id = OUT_BITSTREAM_DATA;


		AACENC_InArgs m_in_args;
		AACENC_OutArgs m_out_args;
		char* m_outofbyte;
		int m_advance_samples = 0;
		char* m_pInputbuf = nullptr;

	};
}