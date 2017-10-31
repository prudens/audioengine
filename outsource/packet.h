#pragma once
// 专门有一个线程处理Message的解压缩。
#include <list>
#include <mutex>
#include "real_audio_common.h"
namespace audio_engine{
	class Packet
	{
	public:
		void version( int v ) { _version = v; }
		void prototype( int type ) { _prototype = type; }
		void compress( int c ) { _compress = c; }
		int compress() const{ return _compress; }
		int version() const{ return _version; }
		int prototype() const{ return _prototype; }
		size_t header_size()const { return sizeof( PacketHeader ); }
		size_t content_length( const void* data )const;
	public:
		BufferPtr Build( BufferPtr buf );
		BufferPtr Parse( BufferPtr buf );

	protected:
		bool ParseHeader( BufferPtr buf );
		void AddHeader( BufferPtr buf );
	private:
		int _version = 1;
		int _prototype = 1;
		int _compress = 0;
	};
}