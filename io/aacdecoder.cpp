#include "aacdecoder.h"
#include <string>
namespace audio_engine
{
	AACDecoder::AACDecoder()
	{
		m_hAACDecoder = aacDecoder_Open( TT_MP4_ADTS, 1 );
		if(!m_hAACDecoder)
		{
			return;
		}
		m_bInit = true;
	}

	AACDecoder::~AACDecoder()
	{
		if(m_bInit)
		{
			aacDecoder_Close( m_hAACDecoder );
		}
	}

	void AACDecoder::Release()
	{
		delete this;
	}

	bool AACDecoder::Decode( void* encodeData, int encLen, int16_t* pcmData, int& pcmLen )
	{
		if(!m_bInit)
		{
			return false;
		}
		UINT size = encLen;
		UCHAR* encData = (UCHAR*)encodeData;
		AAC_DECODER_ERROR err;
		m_bytevalid = size;
		err = aacDecoder_Fill( m_hAACDecoder, &encData, &size, &m_bytevalid );
		if(err != AAC_DEC_OK)
		{
			return false;
		}
		err = aacDecoder_DecodeFrame( m_hAACDecoder, m_outBuf, OUTPUT_BUF_SIZE, 0 );
		if(err != AAC_DEC_OK)
		{
			return false;
		}

		pcmLen = m_frameSize;
		memcpy( pcmData, m_outBuf, pcmLen * 2 );
		return true;
	}
}