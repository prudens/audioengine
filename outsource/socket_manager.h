#pragma once
#include <list>
#include <map>
#include <atomic>
#include <functional>
#include "asio.hpp"

typedef int socket_t;
typedef std::function<void( std::error_code ec, std::size_t length )> ReadHandler;
typedef std::function<void( std::error_code ec, std::size_t length )> WriteHandler;
typedef std::function<void( std::error_code ec, socket_t socket_id )> ConnectHandle;
typedef std::function<bool( std::error_code ec, socket_t socket_id )> AcceptHandle;
class SocketManager
{
public:
    SocketManager();
    ~SocketManager();
public:
    asio::io_context& io_context();
    socket_t Connect( std::string ip, int16_t port );//同步
    socket_t Accept( std::string ip, int16_t port );//同步
    void AsyncConnect( std::string ip, int16_t port, ConnectHandle handle );
    void AsyncAccept( std::string ip, int16_t port, AcceptHandle handle );
    void DisConnect( socket_t socket_id );
    void DisAccept(std::string ip, int16_t port);
    void DestroyTcpSocket( socket_t socket_id );
    int Write( socket_t socket_id, const void* buffer, size_t length );
    int Read( socket_t socket_id, void* buffer, size_t& length );
    void AsyncWrite( socket_t socket_id, const void* buffer, size_t length, WriteHandler handle );
    void AsyncRead( socket_t socket_id, void* buffer, size_t length, ReadHandler handle );
private:
    void DoAccept( std::shared_ptr<asio::ip::tcp::acceptor> accept, AcceptHandle handle );
    SocketManager( const SocketManager& ) = delete;
    SocketManager( const SocketManager&& ) = delete;
    asio::io_context _context;
    std::future<void> _future;
    std::map<int, std::shared_ptr<asio::ip::tcp::socket> > _tcp_list;
    struct stAcceptor
    {
        asio::ip::tcp::endpoint ep;
        std::shared_ptr<asio::ip::tcp::acceptor> accept;
    };
    int _socket_idx = 1;
    std::list<stAcceptor> _acceptor_list;

    
};