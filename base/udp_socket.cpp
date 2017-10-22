#include "udp_socket.h"
#include "asio.hpp"


// class IpAddrImpl:public IpAddr
// {
// public:
// 	IpAddrImpl()
// 	{
// 
// 	}
// 	~IpAddrImpl()
// 	{
// 
// 	}
// 	virtual int  port() const
// 	{
// 		
// 	}
// 	virtual std::string str_ip()const
// 	{
// 
// 	}
// 	virtual uint32_t ip()const
// 	{
// 
// 	}
// 	virtual bool is_v4()const
// 	{
// 		return ep.address().is_v4();
// 	}
// 	asio::ip::udp::endpoint ep;
// };
class UdpSocketImpl : public UdpSocket
{
public:
	UdpSocketImpl(asio::io_context&context):_socket(context)
	{
		
	}
	~UdpSocketImpl()
	{

	}

	virtual void Connect(std::string ip, uint16_t port)
	{
		std::error_code ec;
		asio::ip::udp::endpoint ep(asio::ip::make_address(ip,ec), port);
		_socket.bind(ep);
	}
	virtual std::error_code SendTo(std::string ip, uint16_t port, const void* data, size_t length)
	{
		std::error_code ec;
		asio::ip::udp::endpoint ep(asio::ip::make_address(ip, ec), port);
		_socket.send_to(asio::buffer(data,length),ep, 0,ec);
		return ec;
	}
	virtual std::error_code Send(const void* data, size_t length)
	{
		std::error_code ec;
		_socket.send(asio::buffer(data,length), 0,ec);
	}
	virtual void AsyncSendTo(std::string ip, uint16_t port, const void* data, size_t length, WriteHandler handler)
	{
		std::error_code ec;
		asio::ip::udp::endpoint ep(asio::ip::make_address(ip, ec), port);
		_socket.async_send_to(asio::buffer(data, length), ep, handler);
	}
	virtual void AsyncSend(const void* data, size_t length, WriteHandler handler)
	{
		_socket.async_send(asio::buffer(data,length),handler);
	}

	virtual std::error_code ReceiveFrom(void* data, size_t length, IpAddrPtr & addr)
	{
		asio::ip::udp::endpoint ep;
		std::error_code ec;
		addr.reset();
		_socket.receive_from(asio::buffer(data, length), ep, 0,ec );
		return ec;
	}
	virtual std::error_code Receive(void* data, size_t length)
	{
		std::error_code ec;
		_socket.receive(asio::buffer(data, length), asio::ip::udp::socket::message_peek,ec);
		return ec;
	}

	virtual void AsyncReceiveFrom(void* data, size_t length, ReadHandler handler)
	{
		asio::ip::udp::endpoint ep;
		_socket.async_receive_from(asio::buffer(data, length), ep, [=](std::error_code ec, std::size_t len)
		{
			IpAddrPtr p;
			handler( ec, len, p);
		});
	}
	virtual void AsyncReceive(void* data, size_t length, ReadHandler handler)
	{
		std::error_code ec;
		_socket.async_receive(asio::buffer(data, length), [=](std::error_code ec, std::size_t len)
		{
			IpAddrPtr p;
			handler(ec,len,p);
		});
	}

private:
	asio::ip::udp::socket _socket;
};