#include "audio_resample.h"
#include <algorithm>
namespace audio_engine{
	AudioResample::AudioResample()
	{

	}

	bool AudioResample::ToMono( int16_t*src, int num_samples, int16_t*dst )
	{
		if(!dst || !src || num_samples == 0)
		{
			return false;
		}
		int16_t*pSrc = src;
		int16_t*pDst = dst;
		for(int i = 0, j = 0; i < num_samples; j++, i += 2)
		{
			pDst[j] = ( pSrc[i] + pSrc[i + 1] ) / 2;
		}
		return true;
	}

	bool AudioResample::ToMono( int16_t*src, int num_samples )
	{
		return ToMono( src, num_samples, src );
	}

	bool AudioResample::ToMono( int16_t*src, int16_t num_samples, int16_t*left, int16_t*right )
	{
		if(!src || num_samples == 0 || ( !left && !right ))
		{
			return false;
		}

		for(int i = 0; i < num_samples / 2; i++)
		{
			if(left)
			{
				left[i] = src[i * 2];
			}
			if(right)
			{
				right[i] = src[i * 2 + 1];
			}
		}
		return true;
	}

	bool AudioResample::Tostereo( int16_t*src, int num_samples, int16_t*dst )
	{
		if(src == dst)
		{
			return Tostereo( src, num_samples );
		}
		int16_t*pSrc = src;
		int16_t*pDst = dst;
		for(int i = 0, j = 0; i < num_samples; j += 2, i++)
		{
			pDst[j] = pSrc[i];
			pDst[j + 1] = pSrc[i];
		}
		return true;
	}

	bool AudioResample::Tostereo( int16_t*src, int num_samples )
	{
		if(!src || num_samples == 0)
		{
			return false;
		}
		int16_t *dst = src;
		for(int i = num_samples - 1; i >= 0; i--)
		{
			dst[i * 2 + 1] = src[i];
			dst[i * 2] = src[i];
		}
		return true;
	}

	bool AudioResample::Tostereo( int16_t* left, int16_t*right, int num_samples, int16_t*dst )
	{
		if(left == 0 || right == 0 || num_samples == 0 || dst == 0)
		{
			return false;
		}
		for(int i = 0; i < num_samples; i++)
		{
			dst[i * 2] = left[i];
			dst[i * 2 + 1] = right[i];
		}
		return true;
	}

	bool AudioResample::Reset( int32_t inSampleRate, int16_t inChannel, int32_t outSamplerate, int16_t outChannel )
	{
		m_inSamplerate = inSampleRate;
		m_inChannel = inChannel;
		m_outSamplerate = outSamplerate;
		m_outChannel = outChannel;
		if(m_inSamplerate == 44100)
		{
			inSampleRate = 44000;
		}
		if(outSamplerate == 44100)
		{
			outSamplerate = 44000;
		}
		size_t channel = std::min( inChannel, outChannel );
		m_ResampleImpl.Reset( m_inSamplerate, outSamplerate, channel );
		return false;
	}

	bool AudioResample::Process( int16_t* inBuf, size_t inSamples, int16_t* outBuf, size_t outSamples )
	{
		int16_t*pSrc = inBuf;
		size_t inLen = inSamples;
		if(m_inChannel == 2 && m_outChannel == 1)
		{
			ToMono( inBuf, inSamples );
			pSrc = inBuf;
			inLen = inSamples / 2;
		}
		if(m_inSamplerate == 44100)
		{
			inLen--;
			if(m_outChannel == m_inChannel && m_outChannel == 2)
			{
				inLen--;
			}
		}
		size_t maxLen;
		size_t ret = m_ResampleImpl.Push( pSrc, inLen, outBuf, outSamples, maxLen );
		if(m_inChannel == 1 && m_outChannel == 2)
		{
			Tostereo( outBuf, inSamples, outBuf );
		}
		if(m_outSamplerate == 44100)
		{
			if(m_outChannel == 2)
			{
				outBuf[maxLen - 1] = outBuf[maxLen - 3];
				outBuf[maxLen - 2] = outBuf[maxLen - 3];
			}
			else
			{
				outBuf[maxLen - 1] = outBuf[maxLen - 2];
			}
		}
		return ret == 0;
	}
}