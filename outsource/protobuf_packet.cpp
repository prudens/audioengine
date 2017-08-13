#include "protobuf_packet.h"
#include "./real_audio_client/client_module.h"
#include "user_service.pb.h"
using namespace audio_engine;


ProtoPacket::ProtoPacket( ReadPacket cb)
 :_cb(cb)
{
    _buffer = std::make_shared<Buffer>();
}

BufferPtr ProtoPacket::Build( std::shared_ptr<audio_engine::RAUserMessage> pb )
{
    size_t size = pb->ByteSizeLong();
    size_t packet_len = size + header_size();
    BufferPtr buf;
    buf = std::make_shared<Buffer>(packet_len);
    if ( !pb->SerializePartialToArray( buf->WriteData() + header_size(), size ) )
    {
        buf.reset();
    }
    else
    {
        AddHeader( buf );
    }
    buf->Write( packet_len );
    return buf;
}

void ProtoPacket::Parse( BufferPtr buf )
{
    _buffer->WriteAvailable( buf->ReadAvailable() );
    memcpy( _buffer->WriteData(), buf->ReadData(), buf->ReadAvailable() );
    _buffer->Write( buf->ReadAvailable() );

    while ( _buffer->ReadAvailable() > header_size() )
    {
        if (ParseHeader(_buffer))
        {
            auto length = content_length( _buffer->ReadData() );
            auto pb = std::make_shared<audio_engine::RAUserMessage>();
            if ( pb->ParsePartialFromArray( buf->ReadData() + header_size(), length ) )
            {
                _buffer->Read( length + header_size() );
                if(_cb)_cb(std::error_code(),pb);
            }
        } 
    }
}

