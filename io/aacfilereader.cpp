#include "aacfilereader.h"
#include <string.h>
namespace audio_engine
{
	struct ADTSHeader
	{
		uint16_t syncword;//12
		uint8_t version;//1
		uint8_t layer;//2
		uint8_t protect_absent;//1
		uint8_t profile;//2
		uint8_t sampleindex;//4
		uint8_t private_stream;//1
		uint8_t channel_mode;//3;
		uint8_t originality;//1
		uint8_t home;//1
		uint8_t cr_stream;//1
		uint8_t cr_start;//1
		uint16_t frame_length;//13
		uint16_t buf_fullness;//11
		uint8_t num_frame;//2
		uint16_t crc;//16
	};

	AACFileReader::AACFileReader( const char* filename )
	{
		m_file = fopen( filename, "rb" );
		if(!m_file)
		{
			return;
		}
		m_hAACDecoder = aacDecoder_Open( TT_MP4_ADTS, 1 );
		if(!m_hAACDecoder)
		{
			return;
		}

		AAC_DECODER_ERROR err;
		UINT bytevalid = 0;
		UINT bufsize = 0;
		CStreamInfo* pStreamInfo = nullptr;
		bufsize = fread( m_inBuf, 1, sizeof( m_inBuf ), m_file );
		UCHAR*p = (UCHAR*)m_inBuf;
		bytevalid = bufsize;
		err = aacDecoder_Fill( m_hAACDecoder, &p, &bufsize, &bytevalid );
		if(err != AAC_DEC_OK)
		{
			return;
		}

		err = aacDecoder_DecodeFrame( m_hAACDecoder, m_outBuf, OUTPUT_BUF_SIZE, 0 );
		if(err == AAC_DEC_NOT_ENOUGH_BITS)
		{
			return;
		}
		else if(err != AAC_DEC_OK)
		{
			return;
		}
		m_advace_pos = 0;
		{
			pStreamInfo = aacDecoder_GetStreamInfo( m_hAACDecoder );
			m_nSamplerate = pStreamInfo->sampleRate;
			m_nChannels = pStreamInfo->numChannels;
			m_frameSize = pStreamInfo->frameSize;
		}

		Analyze();
		fseek( m_file, bufsize, SEEK_SET );
		m_bInit = true;
	}

	AACFileReader::~AACFileReader()
	{
		if(m_hAACDecoder)
			aacDecoder_Close( m_hAACDecoder );

		if(m_file)
			fclose( m_file );
	}

	void AACFileReader::Destroy()
	{
		delete this;
	}

	int AACFileReader::SampleRate() const
	{
		return m_nSamplerate;
	}

	size_t AACFileReader::NumChannels() const
	{
		return m_nChannels;
	}

	size_t AACFileReader::NumSamples() const
	{
		return m_nSample;
	}

	size_t AACFileReader::ReadSamples( size_t /*num_samples*/, float* /*samples*/ )
	{
		return 0;
	}

	size_t AACFileReader::ReadSamples( size_t num_samples, int16_t* samples )
	{
		if(!m_bInit)
		{
			return 0;
		}
		if(m_readSample >= m_nSample)
		{
			return 0;
		}
		if(m_advace_pos + num_samples <= OUTPUT_BUF_SIZE)
		{
			memcpy( samples, m_outBuf + m_advace_pos, num_samples * 2 );
			m_advace_pos += num_samples;
			m_readSample += num_samples;
		}
		else
		{
			int left = OUTPUT_BUF_SIZE - m_advace_pos;
			memcpy( samples, m_outBuf + m_advace_pos, left * 2 );
			m_advace_pos += left;
			int need_read = num_samples - left;
			if(ReadFrame())
			{
				memcpy( samples + left, m_outBuf, need_read * 2 );
				m_advace_pos = need_read;
			}
			else
			{
				m_readSample += left;
				return left;
			}
			m_readSample += num_samples;
		}

		return num_samples;
	}

	size_t AACFileReader::RemainSamples() const
	{
		return m_nSample - m_readSample;
	}

	bool AACFileReader::SeekSamples( size_t pos )
	{
		if(!m_bInit)
		{
			return false;
		}
		if(pos >= m_nSample)
		{
			return false;
		}
		int index = pos / m_frameSize / m_nChannels;
		AAC_DECODER_ERROR err = aacDecoder_SetParam( m_hAACDecoder, AAC_TPDEC_CLEAR_BUFFER, 1 );
		if(err != AAC_DEC_OK)
		{
			return false;
		}
		int position = 0;
		Analyze( index, &position );
		m_readSample = index * m_frameSize * m_nChannels;
		fseek( m_file, position, SEEK_SET );
		m_advace_pos = OUTPUT_BUF_SIZE;
		return true;
	}

	bool AACFileReader::SeekTime( double sec )
	{
		if(!m_bInit)
		{
			return false;
		}
		return SeekSamples( static_cast<size_t>( sec * m_nSamplerate * m_nChannels ) );
	}

	bool AACFileReader::SetSpeed( double /*times*/ )
	{
		return false;
	}

	bool AACFileReader::ReadFrame()
	{
		AAC_DECODER_ERROR err;

		UINT bufsize = 0;
		for(;; )
		{
			for(;; )
			{
				err = aacDecoder_DecodeFrame( m_hAACDecoder, m_outBuf, OUTPUT_BUF_SIZE, 0 );
				if(err == AAC_DEC_NOT_ENOUGH_BITS || err == AAC_DEC_TRANSPORT_SYNC_ERROR)
				{
					break;
				}
				else if(err != AAC_DEC_OK)
				{
					break;
				}
				else
				{
					return true;
				}
			}

			if(m_bytevalid == 0)
			{
				bufsize = fread( m_inBuf, 1, sizeof( m_inBuf ), m_file );
				if(bufsize == 0)
				{
					return false;
				}
				m_bytevalid = bufsize;
			}
			UINT startpos = bufsize - m_bytevalid;
			bufsize = m_bytevalid;
			UCHAR*p = (UCHAR*)m_inBuf + startpos;
			err = aacDecoder_Fill( m_hAACDecoder, &p, &bufsize, &m_bytevalid );
			if(err != AAC_DEC_OK)
			{
				printf( "aacDecoder_Fill() failed: 0x%x", err );
				return false;
			}
		}

	}

	bool AACFileReader::Analyze( int indexFrame, int* pos )
	{
		//‭111111111111 0 00 1 01 0111 0 010 0 0 0 0 0000010111001 00010101000 00‬
		fseek( m_file, 0, SEEK_SET );
		uint8_t header[7] = { 0 };
		ADTSHeader aacHeader;
		int index = 0;
		int seek = 0;
		for(;; )
		{
			if(7 != fread( header, 1, 7, m_file ))
			{
				break;
			}
			uint8_t*p = header;
			aacHeader.syncword = *p;
			++p;
			aacHeader.syncword <<= 4;
			aacHeader.syncword += ( *p >> 4 );
			aacHeader.version = ( *p >> 3 ) & 0x1;
			aacHeader.layer = ( *p >> 1 ) & 0x3;
			aacHeader.protect_absent = *p & 0x1;
			++p;
			aacHeader.profile = *p >> 6;//2
			aacHeader.sampleindex = ( *p >> 2 ) & 0x0f;//4
			aacHeader.private_stream = ( *p >> 1 ) & 0x1;
			aacHeader.channel_mode = ( *p ) & 0x1;
			aacHeader.channel_mode <<= 2;
			++p;
			aacHeader.channel_mode += ( *p >> 6 ) & 0x3;//2
			aacHeader.originality = ( *p >> 5 ) & 0x1;//1
			aacHeader.home = ( *p >> 4 ) & 0x1;//1
			aacHeader.cr_stream = ( *p >> 3 ) & 0x1;//1
			aacHeader.cr_start = ( *p >> 2 ) & 0x1;//1
			aacHeader.frame_length = ( *p ) & 0x3;//2
			++p;
			aacHeader.frame_length <<= 8;
			aacHeader.frame_length |= *p;
			aacHeader.frame_length <<= 3;
			++p;
			aacHeader.frame_length |= *p >> 5;//3

			aacHeader.buf_fullness = *p & 0x1f;//5
			++p;
			aacHeader.buf_fullness <<= 6;
			aacHeader.buf_fullness |= *p >> 2;//6
			aacHeader.num_frame = *p & 0x3;
			index++;

			if(aacHeader.syncword != 0xfff)
			{
				return false;
			}
			seek += aacHeader.frame_length;
			if(index == indexFrame && pos)
			{
				*pos = seek;
			}
			fseek( m_file, seek, SEEK_SET );
		}

		fseek( m_file, 0, SEEK_SET );

		m_nSample = index * m_frameSize*m_nChannels;
		return true;
	}
}