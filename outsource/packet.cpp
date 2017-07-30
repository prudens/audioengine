#include "packet.h"
#include <system_error>
#include "socket_manager.h"
#include "base/async_task.h"
#include "./real_audio_client/client_module.h"

bool Packet::ParseHeader( BufferPtr buf )
{
    return true;
}

size_t Packet::content_length( const void* data ) const
{
    PacketHeader* header = (PacketHeader*)data;
    if (header)
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
    PacketHeader* header = (PacketHeader*)buf->data();
    size_t content_length = buf->length() - header_size();
    header->compress = compress();
    header->content_length = content_length;
    header->prototype = prototype();
    header->version = version();
}

BufferPtr Packet::Parse(  BufferPtr buf )
{
     if (ParseHeader(buf))
     {
         return buf;
     }
     else
     {
         return nullptr;
     }
}
