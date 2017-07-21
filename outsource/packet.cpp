#include "packet.h"
#include <system_error>
#include "socket_manager.h"
#include "base/async_task.h"
#include "./real_audio_client/client_module.h"

Packet::Packet()
{

}

Packet::~Packet()
{
}

bool Packet::ParseHeader( PacketHeader* header )
{
    return true;
}

void Packet::Build( int server_type, BufferPtr buf )
{
    auto pack_buf = Pack( server_type, buf );
    if (pack_buf)
    {
        AddHeader( pack_buf );
        Write( server_type, pack_buf );
    }
}


BufferPtr Packet::PullFromBufferPool( size_t capacity )
{
    BufferPtr bufptr;
    _lock_sockets.lock();
    if ( _buffer_pool.empty() )
    {
        bufptr = std::make_shared<Buffer>(capacity);
    }
    else
    {
        bufptr = _buffer_pool.back();
        _buffer_pool.pop_back();
    }
    _lock_sockets.unlock();
    return bufptr;
}

void Packet::PushToBufferPool( BufferPtr ptr )
{
    _lock_sockets.lock();
    _buffer_pool.push_back( ptr );
    _lock_sockets.unlock();
}

void Packet::AddHeader( BufferPtr buf )
{
    PacketHeader* header = (PacketHeader*)buf->data;
    size_t content_length = buf->length - header_size();
    header->compress = compress();
    header->content_length = content_length;
    header->prototype = prototype();
    header->version = version();
}

void Packet::Parse( int server_type, BufferPtr buf )
{
    PacketHeader*header = ( PacketHeader* )buf->data;
    if ( ParseHeader( header ) )
    {
        BufferPtr unpack_buf = UnPack( server_type, buf );
        if ( unpack_buf )
        {
            Read( server_type, unpack_buf->data + header_size(), unpack_buf->length - header_size() );
            PushToBufferPool( unpack_buf );
        }
    }
}

BufferPtr Packet::Pack( int server_type, BufferPtr buf )
{
    return buf;
}

BufferPtr Packet::UnPack( int server_type, BufferPtr buf )
{
    return buf;
}

void Packet::Read( int server_type, void* data, size_t length )
{
    
}

void Packet::Write( int server_type, BufferPtr buf )
{

}
