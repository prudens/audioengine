#include "packet.h"
#include <system_error>
#include "base/async_task.h"
#include "./real_audio_client/client_module.h"
namespace audio_engine{
	bool Packet::ParseHeader( BufferPtr buf )
	{
		if(buf->ReadAvailable() >= content_length( buf->ReadData() ))
		{
			return true;
		}
		return false;
	}

	size_t Packet::content_length( const void* data ) const
	{
		PacketHeader* header = (PacketHeader*)data;
		if(header)
		{
			return header->content_length;
		}
		return 0;
	}

	BufferPtr Packet::Build( BufferPtr buf )
	{
		AddHeader( buf );
		return buf;
	}

	void Packet::AddHeader( BufferPtr buf )
	{
		PacketHeader* header = (PacketHeader*)buf->WriteData();
		header->compress = compress();
		header->content_length = buf->WriteAvailable() - header_size();
		header->prototype = prototype();
		header->version = version();
	}

	BufferPtr Packet::Parse( BufferPtr buf )
	{
		if(ParseHeader( buf ))
		{
			return buf;
		}
		else
		{
			return nullptr;
		}
	}
}