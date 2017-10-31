#pragma once
#include "./include/audiodecoder.h"
#include "codec/g7221/include/g722_1.h"
namespace audio_engine
{
	class G7221Decoder :public AudioDecoder
	{
	public:
		G7221Decoder( int32_t samplerate, int16_t channel );
		virtual void Release();
		virtual bool Decode( void* encodeData, int outLen, int16_t* pcmData, int& inLen )override;
		virtual bool SetBitRate( int bitRate );
	private:
		g722_1_decode_state_t *m_decoder;
		bool    m_init = false;
	};
}