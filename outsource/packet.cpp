#include "packet.h"
#include <iostream>
#include <chrono>
#include "socket_manager.h"
#include "./real_audio_client/client_module.h"
#pragma pack(1)
struct PacketHeader
{
    char version : 2;
    char prototype : 3;
    char compress : 3;
    uint32_t content_length;
};
#pragma  pack()

PacketHandler::PacketHandler(SocketManager* socket_mgr)
{
    _socket_mgr = socket_mgr;
}

PacketHandler::~PacketHandler()
{
    for ( auto v : _sockets )
    {
        _socket_mgr->DisConnect( v.second);
    }
}

size_t PacketHandler::header_size() const
{
    return sizeof( PacketHeader );
}



bool PacketHandler::ParseHeader( PacketHeader* header )
{
    return true;
}

void PacketHandler::Pruduce( int server_type, void* packet, size_t length )
{
    PacketPtr bufptr;
    _lock_sockets.lock();
    if ( _buffer_pool.empty() )
    {
        bufptr = std::make_shared<Packet>();
    }
    else
    {
        bufptr = _buffer_pool.back();
        _buffer_pool.pop_back();
    }

    _lock_sockets.unlock();
    memcpy( bufptr->data + sizeof( PacketHeader ), packet, length );
    bufptr->length = length;
    bufptr->server_type = server_type;
    _write_product_packets.push_back( bufptr );
}


bool PacketHandler::Process()
{
    bool bsleep = true;
    _lock_sockets.lock();
    if ( !_read_consum_packets.empty() )
    {
        PacketPtr packet = _read_consum_packets.front();
        _read_consum_packets.pop_front();
        _lock_sockets.unlock();
        _ConsumHandle( packet, packet->server_type );
        _lock_sockets.lock();
        _buffer_pool.push_back( packet );
        bsleep = false;
    }
    if ( !_write_product_packets.empty() )
    {
        auto packet = _write_product_packets.front();
        _write_product_packets.pop_front();
        _lock_sockets.unlock();
        _ProductHandle( packet, packet->server_type );
        _lock_sockets.lock();
        _buffer_pool.push_back( packet );
        _lock_sockets.unlock();
        bsleep = false;
    }
    else
    {
        _lock_sockets.unlock();
    }
    return bsleep;
}

void PacketHandler::Consum(int server_type)
{
    PacketPtr bufptr;
    _lock_sockets.lock();
    if (_buffer_pool.empty())
    {
        bufptr = std::make_shared<Packet>();
    }
    else
    {
        bufptr = _buffer_pool.back();
        _buffer_pool.pop_back();
    }
    socket_t fd = _sockets[server_type];
    _lock_sockets.unlock();
    _socket_mgr->AsyncRead( fd, bufptr->data, bufptr->capacity, [=] ( std::error_code ec, size_t length )
    {
        if ( !ec )
        {
            PacketHeader* packet = (PacketHeader*)bufptr->data;
            bufptr->length = length;
            if ( ParseHeader( packet ) )
            {
                _read_consum_packets.push_back( bufptr );
            }
            Consum( server_type );
        }
        else
        {
            HandleError(ec, server_type);
        }

    } );
}

void PacketHandler::HandleError(std::error_code ec, int server_type)
{
    if (_ErrorHandle)
    {
        _ErrorHandle(ec,server_type);
    }
}


void PacketHandler::AddServer( int server_type, std::string ip, int port )
{
    _socket_mgr->AsyncConnect( ip, port, [=] ( std::error_code ec, socket_t socket_id )
    {
        if (!ec)
        {
            _lock_sockets.lock();
            _sockets[server_type] = socket_id;
            _lock_sockets.unlock();
            Consum( server_type );
        }
        else
        {
            HandleError( ec, server_type );
        }
    } );
}

void PacketHandler::RemoveServer( int server_type )
{
    _lock_sockets.lock();
    socket_t fd = _sockets[server_type];
    _sockets.erase( server_type );
    _lock_sockets.unlock();
    _socket_mgr->DisConnect( fd );
}

