#pragma once
#include <list>
#include <map>
#include <atomic>
#include <functional>
#include "asio.hpp"
#include "real_audio_common.h"
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
    std::error_code DisAccept( std::string ip, int16_t port );
    void DestroyTcpSocket( socket_t socket_id );
    std::error_code Write( socket_t socket_id, const void* buffer, size_t length );
    std::error_code Read( socket_t socket_id, void* buffer, size_t& length );
    void AsyncWrite( socket_t socket_id, const void* buffer, size_t length, WriteHandler handle );
    void AsyncWrite( socket_t socket_id, BufferPtr buf, WriteHandler handle );
    void AsyncRead( socket_t socket_id, void* buffer, size_t length, ReadHandler handle );
    void AsyncRead( socket_t socket_id, BufferPtr buf, ReadHandler handle );
    bool QuerySocketInfo( socket_t socket_id, std::string& ip, int16_t& port );
    void Cancel( socket_t socket_id );
private:
    void DoAccept( std::shared_ptr<asio::ip::tcp::acceptor> accept, AcceptHandle handle );
    SocketManager( const SocketManager& ) = delete;
    SocketManager( const SocketManager&& ) = delete;
    asio::io_context _io_context;
    asio::io_service::work* _work;
    typedef asio::basic_waitable_timer<std::chrono::steady_clock>  steady_timer;
    steady_timer _timer;
    std::future<void> _future;
    std::map<int, std::shared_ptr<asio::ip::tcp::socket> > _tcp_list;
    int _socket_idx = 1;
    typedef std::shared_ptr<asio::ip::tcp::acceptor> AcceptorPtr;
    std::list<std::shared_ptr<asio::ip::tcp::acceptor>> _acceptor_list;
    std::mutex _lock;

    
};