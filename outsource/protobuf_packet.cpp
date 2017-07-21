#include "protobuf_packet.h"
#include "./real_audio_client/client_module.h"
#include "user_service.pb.h"
using namespace audio_engine;
ProtoPacket::ProtoPacket( PacketHandle *handle)
{
    _packet_handle = handle;
}

ProtoPacket::~ProtoPacket()
{
}

RAUserMessage* ProtoPacket::AllocProtoBuf()
{
    return new RAUserMessage();
}

void ProtoPacket::FreeProtobuf( audio_engine::RAUserMessage* pb )
{
    delete pb;
}

void ProtoPacket::Read( int server_type, void* data, size_t length )
{
    //²»´¦Àí
}

void ProtoPacket::Write( int server_type, BufferPtr buf )
{
    _packet_handle->SendPacket( server_type, buf );
}

BufferPtr ProtoPacket::Pack( int server_type, BufferPtr buf )
{
    BufferPtr newbuf;
    if ( buf->length - header_size() != sizeof( RAUserMessage* ) )
    {
        return newbuf;
    }
    RAUserMessage*pb = nullptr;
    memcpy( (void*)&pb, buf->data + header_size(), sizeof( RAUserMessage* ) );
    if ( !pb )
    {
        return newbuf;
    }
    size_t size = pb->ByteSizeLong();
    newbuf = PullFromBufferPool( size + header_size() );
    newbuf->length = size + header_size();
    if ( !pb->SerializePartialToArray( newbuf->data + header_size(), size ) )
    {
        PushToBufferPool(newbuf);
        newbuf.reset();
    }
    FreeProtobuf( pb );
    return newbuf;
}

BufferPtr ProtoPacket::UnPack( int server_type, BufferPtr buf )
{
    RAUserMessage pb;
    if ( pb.ParsePartialFromArray( buf->data + header_size(), buf->length - header_size() ) )
    {
        _packet_handle->RecvPacket( server_type, &pb );
    }
    PushToBufferPool( buf );
    return nullptr;
}

BufferPtr ProtoPacket::Produce( int server_type, const char* data, size_t length )
{
    BufferPtr bufptr = PullFromBufferPool( length + header_size() );

    memcpy( bufptr->data + header_size(), data, length );
    bufptr->length = length + header_size();
    return bufptr;
}


