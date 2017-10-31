#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "common_defines.h"
namespace audio_engine
{
	class BitBlock
	{
	public:
		BitBlock(uint8_t* block, size_t length)
		{
			_block = block;
			_length = length;
		}
		template<typename T, typename = std::enable_if<std::is_integral<T>::value, T>::type>
		void PushBits(T v, int nbit)
		{
			ASSERT(nbit + bit_idx <= _length * 8);
			for (int i = 0; i < nbit; i++)
			{
				int b = 1 & (v >> (nbit - i - 1));
				_block[bit_idx / 8] |= b << (7 - bit_idx % 8);
				bit_idx++;
			}
		}
		template<typename T, typename = std::enable_if<std::is_integral<T>::value, T>::type>
		void PullBits(T&v, int nbit)
		{
			ASSERT(nbit + bit_idx <= _length * 8);
			for (int i = 0; i < nbit; i++)
			{
				int b = 1 & _block[bit_idx / 8] >> (7 - bit_idx % 8);
				v |= b << (nbit - i - 1);
				bit_idx++;
			}
		}
	private:
		uint8_t* _block;
		size_t _length;
		int bit_idx = 0;
	};
}