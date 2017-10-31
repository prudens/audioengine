#pragma once
#include <string>
#include <memory>
#include <functional>
#include <system_error>
namespace audio_engine
{
	class IpAddr
	{
	public:
		virtual int  port() const = 0;
		virtual uint32_t ip()const = 0;
		virtual std::string str_ip()const = 0;
		virtual bool is_v4()const = 0;
	};
	typedef std::shared_ptr<IpAddr> IpAddrPtr;
	typedef std::function<void( std::error_code ec, std::size_t length, IpAddrPtr )> ReadHandler;
	typedef std::function<void( std::error_code ec, std::size_t length )> WriteHandler;


	class UdpSocket
	{
	public:
		virtual std::error_code Connect( std::string ip, uint16_t port ) = 0;
		virtual std::error_code SendTo( std::string ip, uint16_t port, const void* data, size_t length ) = 0;
		virtual std::error_code Send( const void* data, size_t length ) = 0;
		virtual void AsyncSendTo( std::string ip, uint16_t port, const void* data, size_t length, WriteHandler handler ) = 0;
		virtual void AsyncSend( const void* data, size_t length, WriteHandler handler ) = 0;
		virtual std::error_code ReceiveFrom( void* data, size_t& length, IpAddrPtr & addr ) = 0;
		virtual std::error_code Receive( void* data, size_t& length ) = 0;
		virtual void AsyncReceiveFrom( void* data, size_t length, ReadHandler handler ) = 0;
		virtual void AsyncReceive( void* data, size_t length, ReadHandler handler ) = 0;
	};
	typedef std::shared_ptr<UdpSocket> UdpSocketPtr;
}