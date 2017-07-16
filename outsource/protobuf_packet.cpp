#include "protobuf_packet.h"
#include "./real_audio_client/client_module.h"
ProtobufPacket::ProtobufPacket()
{
    auto socket_mgr = ClientModule::GetInstance()->GetSocketManager();
    _msg_handler = new PacketHandler( socket_mgr );
}

ProtobufPacket::~ProtobufPacket()
{
    delete _msg_handler;
}

