#pragma once
#include "real_audio_common.h"
#include "packet.h"
//Ç°ÏòÉùÃ÷
namespace audio_engine
{
    class RAUserMessage;
}

class PacketHandle
{
public:
    virtual void RecvPacket( int server_type, audio_engine::RAUserMessage* buf ) = 0;
    virtual void SendPacket( int server_type, BufferPtr buf ) = 0;
};

class ProtoPacket: public Packet
{
public:
    explicit ProtoPacket( PacketHandle*handle );
    ~ProtoPacket();
    BufferPtr Produce( int server_type, const char* data, size_t length );
    audio_engine::RAUserMessage* AllocProtoBuf();
    void FreeProtobuf( audio_engine::RAUserMessage* pb);
protected:
    virtual void Read( int server_type, void* data, size_t length );
    virtual void Write( int server_type, BufferPtr buf );
    virtual BufferPtr Pack( int server_type, BufferPtr buf );
    virtual BufferPtr UnPack( int server_type, BufferPtr buf );
private:
    PacketHandle* _packet_handle = nullptr;
};