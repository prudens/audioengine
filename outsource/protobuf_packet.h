#pragma once
#include "packet.h"
class SocketManager;
class PacketHandler;
class ProtobufPacket
{
public:
    explicit ProtobufPacket( );
    ~ProtobufPacket();
    void AddServer();
private:
    PacketHandler* _msg_handler;
};