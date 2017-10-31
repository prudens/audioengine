#include <cassert>
#include <string>
#include <algorithm>
#include "g7721encoder.h"
namespace audio_engine
{
	G7221Encoder::G7221Encoder( int32_t samplerate, int16_t, int bitrate )
	{
		m_encoder = g722_1_encode_init( nullptr, 16000, samplerate );
		if(!m_encoder)
		{
			return;
		}
		SetBitRate( bitrate );
		m_init = true;
		m_frameSize = samplerate / 50;
	}

	void G7221Encoder::Release()
	{
		if(m_init)
		{
			g722_1_encode_release( m_encoder );
		}

		delete this;
	}

	bool G7221Encoder::Encode( int16_t* pcmData, int inLen, char* encodeData, int& outLen )
	{
		size_t pcmLen = inLen / 2;
		size_t advance = 0;
		int32_t outFrameSize = 0;
		uint8_t* pOutData = (uint8_t*)encodeData;
		while(advance < pcmLen)
		{
			if(m_curPos < m_frameSize)
			{
				int a = ( std::min )( (uint32_t)pcmLen - advance, m_frameSize - m_curPos );
				std::copy( pcmData + advance, pcmData + advance + a, m_pcm + m_curPos );
				m_curPos += a;
				advance += a;
			}
			if(m_curPos >= m_frameSize)
			{
				m_curPos -= m_frameSize;
				int encLen = g722_1_encode( m_encoder, m_encbuf, m_pcm, m_frameSize );
				if(encLen == 0)
				{
					return false;
				}
				if(outLen < encLen)
				{
					return false;
				}
				memcpy( encodeData, m_encbuf, outLen );
				pOutData += encLen;
				outFrameSize += encLen;
				outLen -= encLen;
			}
		}
		outLen = outFrameSize;
		return true;
	}

	bool G7221Encoder::SetBitRate( int32_t bitRate )
	{
		return 0 == g722_1_encode_set_rate( m_encoder, bitRate );
	}
}