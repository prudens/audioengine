#include "udp_socket.h"
#include "asio.hpp"

namespace audio_engine
{
	class IpAddrImpl :public IpAddr
	{
	public:
		IpAddrImpl( std::string ip, int16_t port )
		{
			std::error_code ec;
			ep = asio::ip::udp::endpoint( asio::ip::make_address( ip, ec ), port );
		}
		IpAddrImpl(){}
		~IpAddrImpl()
		{

		}
		asio::ip::udp::endpoint& end_point() { return ep; }
		virtual int  port() const override
		{
			return ep.port();
		}
		virtual std::string str_ip()const override
		{
			std::error_code ec;
			return ep.address().to_string( ec );
		}
		virtual uint32_t ip()const override
		{
			if(is_v4())
			{
				return ep.address().to_v4().to_uint();
			}
			return 0;
		}
		virtual bool is_v4()const override
		{
			return ep.address().is_v4();
		}
		asio::ip::udp::endpoint ep;
	};
	class UdpSocketImpl : public UdpSocket
	{
	public:
		UdpSocketImpl( asio::io_context&context ) :_socket( context )
		{

		}
		~UdpSocketImpl()
		{

		}

		virtual std::error_code Connect( std::string ip, uint16_t port )override
		{
			std::error_code ec;
			asio::ip::udp::endpoint ep( asio::ip::make_address( ip, ec ), port );
			if(!ec)
			{
				_socket.connect( ep, ec );
			}
			return ec;
		}
		virtual std::error_code SendTo( std::string ip, uint16_t port, const void* data, size_t length )override
		{
			std::error_code ec;
			asio::ip::udp::endpoint ep( asio::ip::make_address( ip, ec ), port );
			_socket.send_to( asio::buffer( data, length ), ep, 0, ec );
			return ec;
		}
		virtual std::error_code Send( const void* data, size_t length )override
		{
			std::error_code ec;
			_socket.send( asio::buffer( data, length ), 0, ec );
		}
		virtual void AsyncSendTo( std::string ip, uint16_t port, const void* data, size_t length, WriteHandler handler )override
		{
			std::error_code ec;
			asio::ip::udp::endpoint ep( asio::ip::make_address( ip, ec ), port );
			_socket.async_send_to( asio::buffer( data, length ), ep, handler );
		}
		virtual void AsyncSend( const void* data, size_t length, WriteHandler handler )override
		{
			_socket.async_send( asio::buffer( data, length ), handler );
		}

		virtual std::error_code ReceiveFrom( void* data, size_t& length, IpAddrPtr & addr )override
		{
			auto ep = std::make_shared<IpAddrImpl>();
			std::error_code ec;
			size_t len = _socket.receive_from( asio::buffer( data, length ), ep->end_point(), 0, ec );
			length = len;
			addr = ep;
			return ec;
		}
		virtual std::error_code Receive( void* data, size_t& length )override
		{
			std::error_code ec;
			size_t len = _socket.receive( asio::buffer( data, length ), asio::ip::udp::socket::message_peek, ec );
			length = len;
			return ec;
		}

		virtual void AsyncReceiveFrom( void* data, size_t length, ReadHandler handler )override
		{
			auto ep = std::make_shared<IpAddrImpl>();
			_socket.async_receive_from( asio::buffer( data, length ), ep->end_point(), [=]( std::error_code ec, std::size_t len )
			{
				handler( ec, len, ep );
			} );
		}
		virtual void AsyncReceive( void* data, size_t length, ReadHandler handler )override
		{
			std::error_code ec;
			_socket.async_receive( asio::buffer( data, length ), [=]( std::error_code ec, std::size_t len )
			{
				IpAddrPtr p;
				handler( ec, len, p );
			} );
		}

	private:
		asio::ip::udp::socket _socket;
	};
}