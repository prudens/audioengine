#include <cassert>
#include <string>
#include <cstdint>
#include "g7221decoder.h"
#include "codec/g7221/include/g722_1.h"

namespace audio_engine{
	G7221Decoder::G7221Decoder( int32_t samplerate, int16_t )
	{
		m_decoder = g722_1_decode_init( nullptr, 16000, samplerate );
		if(!m_decoder)
		{
			return;
		}
		m_init = true;
	}

	void G7221Decoder::Release()
	{
		if(m_init)
		{
			g722_1_decode_release( m_decoder );
		}

		delete this;
	}

	bool G7221Decoder::Decode( void* encodeData, int inLen, int16_t* pcmData, int &outLen )
	{
		outLen = g722_1_decode( m_decoder, pcmData, (uint8_t*)encodeData, inLen );
		if(outLen == 0)
		{
			return false;
		}
		outLen *= 2;
		return true;
	}

	bool G7221Decoder::SetBitRate( int bitRate )
	{
		return 0 == g722_1_decode_set_rate( m_decoder, bitRate );
	}

}