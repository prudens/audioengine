#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <system_error>

#include "asio.hpp"
class TcpSocket;
typedef std::shared_ptr<TcpSocket> TcpSocketPtr;

typedef std::function<void(std::error_code, std::size_t )> ReadHandler;
typedef std::function<void(std::error_code, std::size_t )> WriteHandler;
typedef std::function<void(std::error_code)> ConnectHandle;
typedef std::function<void(std::error_code, TcpSocketPtr)> AcceptHandle;


class TcpSocket
{
public:
	virtual std::error_code Connect(std::string ip, int16_t port) = 0;
	virtual std::error_code DisConnect() = 0;
	virtual std::error_code Write( const void* buffer, size_t length) = 0;
	virtual std::error_code Read(void* buffer, size_t& length) = 0;
	virtual void            AsyncConnect(std::string ip, int16_t port, ConnectHandle handle) = 0;
	virtual void            AsyncWrite( const void* buffer, size_t length, WriteHandler handle) = 0;
	virtual void            AsyncRead( void* buffer, size_t length, ReadHandler handle) = 0;
	virtual std::error_code QuerySocketInfo( std::string& ip, int16_t& port) = 0;
};

class TcpAcceptor
{
public:
	virtual std::error_code Accept(TcpSocketPtr& socket) = 0;
	virtual void AsyncAccept( AcceptHandle handle) = 0;
	virtual std::error_code DisAccept() = 0;
};
typedef std::shared_ptr<TcpAcceptor> TcpAcceptorPtr;

class TcpFactory
{
public:
	virtual TcpSocketPtr CreateTcpConnection( std::string local_ip = "127.0.0.1",
		                                          int16_t local_port = 0 ) = 0;
	virtual TcpSocketPtr CreateTcpConnection( uint32_t local_ip, int16_t local_port ) = 0;
	virtual TcpAcceptorPtr   CreateTcpAcceptr( std::string local_ip = {},
		                                       int16_t local_port = 0 ) = 0;
};
typedef std::shared_ptr<TcpFactory> TcpFactoryPtr;
TcpFactoryPtr CreateTcpFactory(asio::io_context& context);