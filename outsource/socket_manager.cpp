#include "socket_manager.h"
#include "asio/error_code.hpp"
SocketManager::SocketManager()
:_timer(_io_context)
{
    _work = new asio::io_service::work( _io_context );
    _future = std::async( [&] () { 
        std::error_code ec;
        _io_context.reset();
        _io_context.run(ec);
        if (ec)
        {
            printf(ec.message().c_str());
        }
    } );
}

SocketManager::~SocketManager()
{
    delete _work;
    _io_context.stop();
    _future.get();
}

asio::io_context& SocketManager::io_context()
{
    return _io_context;
}

socket_t SocketManager::Connect( std::string ip, int16_t port )
{
    using namespace asio::ip;
    socket_t sid = 0;
    auto s = std::make_shared<tcp::socket>( _io_context );
    tcp::endpoint ep( asio::ip::make_address( ip ), port );
    std::error_code ec;
    s->connect( ep, ec );
    if ( !ec )
    {
        _lock.lock();
        _tcp_list[++_socket_idx] = s;
        _lock.unlock();
        sid = _socket_idx;
    }
    return sid;
}

socket_t SocketManager::Accept( std::string ip, int16_t port )
{
    socket_t fd = 0;
    using namespace asio::ip;
    std::error_code ec;
    auto s = std::make_shared<tcp::socket>( _io_context );
    asio::ip::tcp::endpoint ep( asio::ip::make_address( ip ), port );
    auto it = std::find_if( _acceptor_list.begin(), _acceptor_list.end(),
                  [&ep] ( AcceptorPtr ptr ) { return ptr->local_endpoint() == ep; } );
    if ( it != _acceptor_list.end() )
    {
        (*it)->accept( *s, ec);
        if (!ec)
        {
            fd = ++_socket_idx;
            _lock.lock();
            _tcp_list[fd] = std::move( s );
            _lock.unlock();
        }
    }
    else
    {
        auto a = std::make_shared<tcp::acceptor>( _io_context, ep );
        _acceptor_list.push_back( std::move( a ) );
        a->accept( *s, ec );
        if ( !ec )
        {
            fd = ++_socket_idx;
            _lock.lock();
            _tcp_list[fd] = s;
            _lock.unlock();
        }
        else
        {
            // log
        }
    }
    return fd;
}

void SocketManager::AsyncConnect( std::string ip, int16_t port, ConnectHandle handle )
{
    using namespace asio::ip;
    std::error_code ec;
    auto s = std::make_shared<tcp::socket>( _io_context );
    tcp::endpoint ep( asio::ip::make_address( ip,ec ), port );
    if (ec)
    {
        handle(ec,0);
    }
    s->async_connect( ep, [=] ( const asio::error_code& ec )
    {
        if ( !ec )
        {
            _lock.lock();
            _tcp_list[++_socket_idx] = s;
            _lock.unlock();
            handle( ec, _socket_idx );
        }
        else
        {
            s->cancel();
            s->close();
            handle( ec, 0 );
        }
    } );
}

void SocketManager::AsyncAccept( std::string ip, int16_t port, AcceptHandle handle )
{
    using namespace asio::ip;
    tcp::endpoint ep( asio::ip::make_address( ip ), port );

    auto it = std::find_if( _acceptor_list.begin(), _acceptor_list.end(), [&ep] ( AcceptorPtr ptr ) { return ptr->local_endpoint() == ep; } );
    if (it != _acceptor_list.end())
    {
        DoAccept( *it, handle );
        return;
    }
   
    auto a = std::make_shared<tcp::acceptor>( _io_context, ep );
    _acceptor_list.push_back( a );
    DoAccept( a, handle );

}

void SocketManager::DisConnect( socket_t socket_id )
{
    DestroyTcpSocket( socket_id );
}


std::error_code SocketManager::DisAccept( std::string ip, int16_t port )
{
    using namespace asio::ip;
    std::error_code ec;

    asio::ip::tcp::endpoint ep( asio::ip::make_address( ip, ec ), port );
    if (!ec)
    {
        return ec;
    }
    auto it = find_if( _acceptor_list.begin(), _acceptor_list.end(),
                       [&] ( const AcceptorPtr& a ) { return a->local_endpoint(ec) == ep; } );
    if ( it != _acceptor_list.end() )
    {
        (*it)->close( ec );
        _acceptor_list.erase( it );
    }
    return ec;
}

void SocketManager::DoAccept( std::shared_ptr<asio::ip::tcp::acceptor> accept, AcceptHandle handle )
{
    using namespace asio::ip;
    auto s = std::make_shared<tcp::socket>( _io_context );
    accept->async_accept( *s, [=] ( asio::error_code ec )
    {
        bool repeat = true;
        if ( !ec )
        {
            _lock.lock();
            _tcp_list[++_socket_idx] = s;
            _lock.unlock();
            repeat = handle( ec, _socket_idx );
        }
        else
        {
            repeat = handle( ec, 0 );
        }
        if ( repeat )
        {
            DoAccept( accept, handle );
        }
    } );
}

void SocketManager::DestroyTcpSocket( socket_t socket_id )
{
    std::unique_lock<std::mutex> lg(_lock);
    auto it = _tcp_list.find( socket_id );
    if (it == _tcp_list.end())
    {
        return;
    }
    asio::error_code ec;
    it->second->close(ec);
    _tcp_list.erase( it );
}

std::error_code SocketManager::Write( socket_t socket_id, const void* data, size_t length )
{
    using namespace asio::ip;
    std::error_code ec;
    _lock.lock();
    auto s = _tcp_list[socket_id];
    _lock.unlock();
    if (!s)
    {
        return ec;
    }
    length = asio::write( *s, asio::buffer( data, length ), ec);
    return ec;
}

std::error_code SocketManager::Read( socket_t socket_id, void* data, size_t& length )
{
    using namespace asio::ip;
    std::error_code ec;
    _lock.lock();
    auto s = _tcp_list[socket_id];
    _lock.unlock();
    if (!s)
    {
        return ec;
    }
    length = s->read_some( asio::buffer( data, length ), ec );
    return ec;
}

void SocketManager::AsyncWrite( socket_t socket_id, const void* data, size_t length, WriteHandler handle )
{
    using namespace asio::ip;
    std::error_code ec;
    _lock.lock();
    auto s = _tcp_list[socket_id];
    _lock.unlock();
    if (!s)
    {
        ec = asio::error::not_socket;
        handle(ec,0);
        return;
    }
    asio::async_write( *s, asio::buffer( data, length ),
                       [=] ( asio::error_code ec, std::size_t len )
    {
        if ( len < length && !ec)//一次没写完就继续写。
        {
            AsyncWrite( socket_id, (char*)data + len, length - len, handle );
        }
        else
        {
            handle( ec, len );
        }
    } );
}

void SocketManager::AsyncWrite( socket_t socket_id, BufferPtr buf, WriteHandler handle )
{
    AsyncWrite( socket_id, buf->RawData(), buf->RawLength(), handle );
}

void SocketManager::AsyncRead( socket_t socket_id, void* data, size_t length, ReadHandler handle )
{ 
    using namespace asio::ip;
    std::error_code ec;
    _lock.lock();
    auto s = _tcp_list[socket_id];
    _lock.unlock();
    if (!s)
    {
        return;
    }
    s->async_read_some( asio::buffer( data, length ),
                       [=] ( asio::error_code ec, std::size_t len )
    {
        handle( ec, len );
    } );
}

void SocketManager::AsyncRead( socket_t socket_id, BufferPtr buf, ReadHandler handle )
{
    using namespace asio::ip;
    std::error_code ec;
    _lock.lock();
    auto s = _tcp_list[socket_id];
    _lock.unlock();
    if ( !s )
    {
        return;
    }
    // read tcp packet header
    asio::async_read( *s, asio::buffer( buf->RawData(), RAW_HEADER_SIZE ),
                        [=] ( asio::error_code ec, size_t length)
    {
        if (ec)
        {
            handle( ec, length );
            return;
        }
        int32_t v = 0;
        CharToInt( buf->RawData(), v );
        // read tcp packet content
        asio::async_read( *s, asio::buffer( buf->data(), v), [=]( asio::error_code ec, size_t length )
        {
            buf->length( length );
            handle(ec, length);

        } );
    } );
}

bool SocketManager::QuerySocketInfo( socket_t socket_id, std::string& ip, int16_t& port )
{
    std::unique_lock<std::mutex> lg( _lock );
    auto it = _tcp_list.find( socket_id );
    if (  it == _tcp_list.end())
    {
        return false;
    }
    std::error_code ec;
    auto ep = it->second->remote_endpoint( ec );
    if ( ec )
    {
        return false;
    }
    ip = ep.address().to_string(ec);
    if ( ec )
    {
        return false;
    }
    port = ep.port();
    return true;

}

void SocketManager::Cancel( socket_t socket_id )
{
    std::unique_lock<std::mutex> lg( _lock );
    auto s = _tcp_list[socket_id];
    std::error_code ec;
    s->cancel( ec );
}
