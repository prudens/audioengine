#pragma once
#include <system_error>
#include <functional>
#include "real_audio_common.h"
#include "packet.h"
//Ç°ÏòÉùÃ÷
namespace audio_engine
{
    class RAUserMessage;
}
using ReadPacket = std::function<void( std::error_code ec, std::shared_ptr<audio_engine::RAUserMessage> pb ) >;
class ProtoPacket: public Packet
{
public:
    ProtoPacket( ReadPacket cb );
    ~ProtoPacket() = default;
    BufferPtr Build( std::shared_ptr<audio_engine::RAUserMessage> pb );
    void Parse( BufferPtr buf );
private:
    ReadPacket _cb;
    BufferPtr _buffer;
};