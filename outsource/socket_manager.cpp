#include "socket_manager.h"

SocketManager::SocketManager()
{
    _future = std::async( [&] () { _context.run(); } );
}

SocketManager::~SocketManager()
{
    _context.stop();
    _future.get();
}

asio::io_context& SocketManager::io_context()
{
    return _context;
}

socket_t SocketManager::Connect( std::string ip, int16_t port )
{
    using namespace asio::ip;
    socket_t sid = 0;
    auto s = std::make_shared<tcp::socket>( _context );
    tcp::endpoint ep( asio::ip::make_address( ip ), port );
    std::error_code ec;
    s->connect( ep, ec );
    if ( !ec )
    {
        _tcp_list[++_socket_idx] = s;
        sid = _socket_idx;
    }
    return sid;
}

socket_t SocketManager::Accept( std::string ip, int16_t port )
{
    using namespace asio::ip;
    std::error_code ec;
    auto s = std::make_shared<tcp::socket>( _context );
    asio::ip::tcp::endpoint ep( asio::ip::make_address( ip ), port );
    for (auto it = _acceptor_list.begin(); it != _acceptor_list.end(); ++it)
    {
        if (it->ep == ep)
        {
            it->accept->accept(*s,ec);
            if (!ec)
            {
                _tcp_list[++_socket_idx] = std::move(s);
                return _socket_idx;
            }
            else
            {
                return 0;
            }
        }
    }

    try
    {
        stAcceptor ac;
        auto a = std::make_shared<tcp::acceptor>( _context, ep );
        a->accept( *s, ec );
        if (!ec)
        {
            _tcp_list[++_socket_idx] = s;

            return _socket_idx;
        }
        else
        {
            // log
        }
        ac.ep = ep;
        ac.accept = a;
        _acceptor_list.push_back( std::move( ac ) );
    }
    catch ( ... )
    {
        return 0;
    }
    return 0;
}

void SocketManager::AsyncConnect( std::string ip, int16_t port, ConnectHandle handle )
{
    using namespace asio::ip;
    auto s = std::make_shared<tcp::socket>( _context );
    tcp::endpoint ep( asio::ip::make_address( ip ), port );
    std::error_code ec;
    s->async_connect( ep, [=] ( const asio::error_code& ec )
    {
        if ( !ec )
        {
            _tcp_list[++_socket_idx] = s;
            handle( ec, _socket_idx );
        }
        else
        {
            handle( ec, 0 );
        }
    } );

}

void SocketManager::AsyncAccept( std::string ip, int16_t port, AcceptHandle handle )
{
    using namespace asio::ip;
    std::error_code ec;

    asio::ip::tcp::endpoint ep( asio::ip::make_address( ip ), port );
    for ( auto it = _acceptor_list.begin(); it != _acceptor_list.end(); ++it )
    {
        if ( it->ep == ep )
        {
            DoAccept( it->accept, handle );
            return;
        }
    }
    try
    {
        auto a = std::make_shared<tcp::acceptor>( _context );
        stAcceptor accpt;
        accpt.ep = ep;
        accpt.accept = a;
        _acceptor_list.push_back( accpt );
        DoAccept( a, handle );
    }
    catch ( ... )
    {

    }
}

void SocketManager::DisConnect( socket_t socket_id )
{
    using namespace asio::ip;
    std::error_code ec;
    auto it = _tcp_list.find( socket_id );
    if (it == _tcp_list.end())
    {
        return;
    }
    if (it->second)
    {
        it->second->close(ec);
    }
    _tcp_list.erase( it );
}


void SocketManager::DisAccept( std::string ip, int16_t port )
{
    using namespace asio::ip;
    std::error_code ec;

    asio::ip::tcp::endpoint ep( asio::ip::make_address( ip ), port );
    auto it = find_if( _acceptor_list.begin(), _acceptor_list.end(), [=] ( const stAcceptor& a ) { return a.ep == ep; } );
    if ( it != _acceptor_list.end() )
    {
        it->accept->close( ec );
        _acceptor_list.erase( it );
    }
}

void SocketManager::DoAccept( std::shared_ptr<asio::ip::tcp::acceptor> accept, AcceptHandle handle )
{
    using namespace asio::ip;
    std::error_code ec;
    auto s = std::make_shared<tcp::socket>( _context );
    accept->async_accept( *s, [=] ( asio::error_code ec )
    {
        bool repeat = true;
        if ( !ec )
        {
            _tcp_list[++_socket_idx] = s;
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
    auto it = _tcp_list.find( socket_id );
    if (it == _tcp_list.end())
    {
        return;
    }
    asio::error_code ec;
    it->second->close(ec);
    _tcp_list.erase( it );
}

int SocketManager::Write( socket_t socket_id, const void* data, size_t length )
{
    using namespace asio::ip;
    std::error_code ec;
    auto s = _tcp_list[socket_id];
    length = asio::write( *s, asio::buffer( data, length ), ec);
    return length;
}

int SocketManager::Read( socket_t socket_id, void* data, size_t& length )
{
    using namespace asio::ip;
    std::error_code ec;
    auto s = _tcp_list[socket_id];
    length = s->read_some( asio::buffer( data, length ), ec );
    return length;
}

void SocketManager::AsyncWrite( socket_t socket_id, const void* data, size_t length, WriteHandler handle )
{
    using namespace asio::ip;
    std::error_code ec;
    auto s = _tcp_list[socket_id];
    asio::async_write( *s, asio::buffer( data, length ),
                       [=] ( asio::error_code ec, std::size_t len )
    {
        handle( ec, len );
    } );
}

void SocketManager::AsyncRead( socket_t socket_id, void* data, size_t length, ReadHandler handle )
{ 
    using namespace asio::ip;
    std::error_code ec;
    auto s = _tcp_list[socket_id];
    s->async_read_some( asio::buffer( data, length ),
                       [=] ( asio::error_code ec, std::size_t len )
    {
        handle( ec, len );
    } );
}
