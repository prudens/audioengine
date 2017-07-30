#include "protobuf_packet.h"
#include "./real_audio_client/client_module.h"
#include "user_service.pb.h"
using namespace audio_engine;


ProtoPacket::ProtoPacket( BufferPool* pool )
    :_buffer_pool(pool)
{
}

BufferPtr ProtoPacket::Build( std::shared_ptr<audio_engine::RAUserMessage> pb )
{
    size_t size = pb->ByteSizeLong();
    size_t packet_len = size + header_size();
    BufferPtr buf;
    if ( _buffer_pool )
        buf = _buffer_pool->PullFromBufferPool( packet_len );
    else
        buf = std::make_shared<Buffer>(packet_len);

    buf->length( packet_len );
    if ( !pb->SerializePartialToArray( buf->data() + header_size(), size ) )
    {
        _buffer_pool->PushToBufferPool( buf );
        buf.reset();
    }
    else
    {
        AddHeader( buf );
    }
    return buf;
}

std::shared_ptr<audio_engine::RAUserMessage> ProtoPacket::Parse( BufferPtr buf )
{
    if ( ParseHeader(buf) )
    {
        auto pb = std::make_shared<audio_engine::RAUserMessage>();
        if ( pb->ParsePartialFromArray( buf->data() + header_size(), buf->length() - header_size() ) )
        {
            return pb;
        }
        
    }
    return nullptr;
}

