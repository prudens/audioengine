#pragma once
#include "real_audio_common.h"
#include "packet.h"
//Ç°ÏòÉùÃ÷
namespace audio_engine
{
    class RAUserMessage;
}

class ProtoPacket: public Packet
{
public:
    ProtoPacket(BufferPool* pool = nullptr);
    ~ProtoPacket() = default;
    BufferPtr Build( std::shared_ptr<audio_engine::RAUserMessage> pb );
    std::shared_ptr<audio_engine::RAUserMessage> Parse( BufferPtr buf );
private:
    BufferPool* _buffer_pool = nullptr;
};