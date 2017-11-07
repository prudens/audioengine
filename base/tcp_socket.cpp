#include "tcp_socket.h"
#include <mutex>
namespace audio_engine
{
	class TcpSocketImpl :public TcpSocket
	{
		struct  BufferCache
		{
			const void* buffer = nullptr;
			size_t length = 0;
			WriteHandler handler;
		};
	public:
		TcpSocketImpl( asio::io_context& context, asio::ip::tcp::endpoint local_ep )
			:_socket( context ), _local_ep( local_ep )
		{
			std::error_code ec;

		}
		~TcpSocketImpl()
		{
			std::error_code ec;
			_socket.close( ec );
		}
	public:
		virtual std::error_code Connect( std::string ip, int16_t port )override
		{
			using namespace asio::ip;
			std::error_code ec;
			tcp::endpoint ep( asio::ip::make_address( ip ), port );
			_socket.open( _local_ep.protocol() );
			_socket.bind( _local_ep, ec );
			_socket.connect( ep, ec );

			return ec;
		}
		virtual std::error_code DisConnect()
		{
			std::error_code ec;
			_socket.shutdown( asio::ip::tcp::socket::shutdown_receive, ec );
			return ec;
		}
		virtual void AsyncConnect( std::string ip, int16_t port, ConnectHandle handle )override
		{
			using namespace asio::ip;
			std::error_code ec;
			tcp::endpoint ep( asio::ip::make_address( ip, ec ), port );
			if(ec)
			{
				handle( ec );
				return;
			}

			_socket.async_connect( ep, [=]( const asio::error_code& ec )
			{
				handle( ec );
			} );
		}
		virtual std::error_code Write( const void* buffer, size_t length )override
		{
			if(_writing)
			{
				printf( "不能在执行异步写操作还未完成的时候，执行同步写操作" );
				assert( false );
			}
			std::error_code ec;
			length = asio::write( _socket, asio::buffer( buffer, length ), ec );
			return ec;
		}
		virtual std::error_code Read( void* buffer, size_t& length )override
		{
			std::error_code ec;
			_socket.read_some( asio::buffer( buffer, length ), ec );
			return ec;
		}
		virtual void  AsyncWrite( const void* buffer, size_t length, WriteHandler handle )override
		{
			BufferCache bc;
			bc.buffer = buffer;
			bc.length = length;
			bc.handler = handle;
			_mutex.lock();
			_buffers.push_back( bc );
			if(!_writing)
			{
				_writing = true;
				bc = _buffers.front();
				_buffers.pop_front();
			}
			_mutex.unlock();
			if(bc.buffer)
			{
				DoAsyncWrite( bc.buffer, bc.length, bc.handler );
			}

		}

		void DoAsyncWrite( const void* buffer, size_t length, WriteHandler handle )
		{
			asio::async_write( _socket, asio::buffer( buffer, length ),
				[=]( asio::error_code ec, std::size_t len )
			{
				handle( ec, len );
				BufferCache bc;
				_mutex.lock();
				if(!_buffers.empty())
				{
					bc = std::move( _buffers.front() );
					_buffers.pop_front();
				}
				else
				{
					_writing = false;
				}
				_mutex.unlock();
				if(bc.buffer)
				{
					DoAsyncWrite( bc.buffer, bc.length, bc.handler );
				}

			} );
		}
		virtual void AsyncRead( void* buffer, size_t length, ReadHandler handle )override
		{
			_socket.async_read_some( asio::buffer( buffer, length ),
				[=]( asio::error_code ec, std::size_t len )
			{
				handle( ec, len );
			} );
		}
		virtual std::error_code QuerySocketInfo( std::string& ip, int16_t& port )override
		{
			std::error_code ec;
			auto ep = _socket.remote_endpoint( ec );
			if(ec)
			{
				return ec;
			}
			ip = ep.address().to_string( ec );
			if(ec)
			{
				return ec;
			}
			port = ep.port();
			return ec;
		}
	public:
		asio::ip::tcp::socket& GetSocket()
		{
			return _socket;
		}
	private:
		asio::ip::tcp::socket _socket;
		asio::ip::tcp::endpoint _local_ep;

		std::list<BufferCache> _buffers;
		bool _writing = false;
		std::mutex _mutex;
	};
	typedef std::shared_ptr<TcpSocketImpl> TcpSocketImplPtr;

	class TcpAcceptorImpl :public TcpAcceptor
	{
	public:
		TcpAcceptorImpl( asio::io_context& context, asio::ip::tcp::endpoint ep ) :_context( context ), _acceptor( context, ep )
		{
		}
		~TcpAcceptorImpl()
		{
			std::error_code ec;
			_acceptor.close( ec );
		}
	public:
		virtual std::error_code Accept( TcpSocketPtr & socket )override
		{
			TcpSocketImplPtr s = std::make_shared<TcpSocketImpl>( _context, asio::ip::tcp::endpoint() );
			using namespace asio::ip;
			std::error_code ec;
			_acceptor.accept( s->GetSocket(), ec );
			if(!ec)
				socket = s;
			return ec;

		}
		virtual void AsyncAccept( AcceptHandle handle )override
		{
			using namespace asio::ip;
			std::error_code ec;
			TcpSocketImplPtr s = std::make_shared<TcpSocketImpl>( _context, asio::ip::tcp::endpoint() );

			_acceptor.async_accept( s->GetSocket(), [=]( std::error_code ec ) mutable
			{
				handle( ec, s );
			} );
		}
		virtual std::error_code DisAccept()override
		{
			std::error_code ec;
			_acceptor.cancel( ec );
			_acceptor.close( ec );
			return ec;
		}
	private:
		asio::io_context& _context;
		asio::ip::tcp::acceptor _acceptor;
	};


	class TcpFactoryImpl :public TcpFactory
	{
	public:
		TcpFactoryImpl( asio::io_context&context ) :_context( context )
		{

		}
		~TcpFactoryImpl()
		{

		}
		virtual TcpSocketPtr CreateTcpConnection( std::string local_ip,
			int16_t local_port = 0 )override
		{
			std::error_code ec;
			asio::ip::tcp::endpoint ep;
			if(local_ip.empty())
			{
				ep = asio::ip::tcp::endpoint( asio::ip::tcp::v4(), local_port );
			}
			else
			{
				ep = asio::ip::tcp::endpoint( asio::ip::make_address( local_ip ), local_port );
			}

			auto p = std::make_shared<TcpSocketImpl>( _context, ep );
			return p;

		}
		virtual TcpSocketPtr CreateTcpConnection( uint32_t local_ip, int16_t local_port )
		{
			local_ip = 0;
			std::error_code ec;
			asio::ip::tcp::endpoint ep;
			if(local_ip == 0)
			{
				ep = asio::ip::tcp::endpoint( asio::ip::tcp::v4(), local_port );
			}
			else
			{
				ep = asio::ip::tcp::endpoint( asio::ip::address_v4( local_ip ), local_port );
			}

			auto p = std::make_shared<TcpSocketImpl>( _context, ep );
			return p;
		}
		virtual TcpAcceptorPtr   CreateTcpAcceptr( std::string local_ip = {},
			int16_t local_port = 0 )
		{
			std::error_code ec;
			asio::ip::tcp::endpoint ep;
			if(local_ip.empty())
			{
				ep = asio::ip::tcp::endpoint( asio::ip::tcp::v4(), local_port );
			}
			else
			{
				ep = asio::ip::tcp::endpoint( asio::ip::make_address( local_ip ), local_port );
			}
			return std::make_shared<TcpAcceptorImpl>( _context, ep );

		}
	private:
		asio::io_context& _context;
	};

	TcpFactoryPtr CreateTcpFactory( asio::io_context& context )
	{
		return std::make_shared<TcpFactoryImpl>( context );
	}

}