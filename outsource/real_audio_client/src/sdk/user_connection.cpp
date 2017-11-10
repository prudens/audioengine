#include "user_connection.h"

#include "client_module.h"
#include "base/async_task.h"
#include "base/common_defines.h"
namespace audio_engine{
	using namespace std::chrono;
	UserService::UserService()
		:_proto_packet( std::bind( &UserService::RecvPacket, this, std::placeholders::_1, std::placeholders::_2 ) )
		,_timer( ClientModule::GetInstance()->GetTimerThread())
	{
		_task = new AsyncTask(ClientModule::GetInstance()->GetThreadPool());
		_timer.AddTask( 100ms, std::bind(&UserService::TimerLoop,this));
	}

	UserService::~UserService()
	{
		delete _task;
	}

	void UserService::ConnectServer( std::string ip, int port )
	{
		_sns_mutex.lock();
		_req_packet_list.clear();
		_sns_mutex.unlock();
		auto self = shared_from_this();
		TcpFactory* fac = ClientModule::GetInstance()->GetTcpFactory();
		_tcp_socket = fac->CreateTcpConnection( "", 0 );
		_proto_packet.Reset();
		ASSERT( _tcp_socket );
		_tcp_socket->AsyncConnect( ip, port, [=]( std::error_code ec )
		{
			if(!ec)
			{
				self->Read();
			}
			_task->AddTask( [=]
			{
				self->_HandleConnect(ec);
			} );
		} );
	}

	void UserService::DisconnectServer()
	{
		if(_tcp_socket)
		{
			_tcp_socket->DisConnect();
			_tcp_socket.reset();
			_sns_mutex.lock();
			_req_packet_list.clear();
			_sns_mutex.unlock();
		}

	}

	bool UserService::IsConnectServer()
	{
		return _tcp_socket != nullptr;
	}

	void UserService::Read( )
	{
		if(!_tcp_socket)
		{
			return;
		}
		auto buf = std::make_shared<Buffer>();
		auto self = shared_from_this();
		_tcp_socket->AsyncRead( buf->WriteData(), buf->WriteAvailable(),
			[=]( std::error_code ec, size_t length )
		{
			if(!ec)
			{
				buf->Write( length );
				self->_task->AddTask( [=]
				{
					self->_proto_packet.Parse( buf );
				} );
				Read( );
			}
			else
			{
				self->_task->AddTask( [=]
				{
					self->_HandleError( ec );
				} );
			}
		} );
	}

	void UserService::Write( BufferPtr buf )
	{
		if(!_tcp_socket)
		{
			return;
		}
		auto self = shared_from_this();
		_tcp_socket->AsyncWrite( buf->ReadData(), buf->ReadAvailable(),
			[=]( std::error_code ec, std::size_t length )
		{
			if(ec)
			{
				self->_task->AddTask( [=]
				{
					self->_HandleError( ec );
				} );
			}
		} );
	}

	void UserService::TimerLoop()
	{
		{
			std::lock_guard<std::mutex> lock( _sns_mutex );
			auto ts = TimeStamp();
			for(auto it = _req_packet_list.begin(); it != _req_packet_list.end();)
			{
				auto wait = it->second;
				if(wait->timeout + wait->calltime < ts)
				{
					_task->AddTask( [wait]
					{
						wait->cb( wait->pb, std::make_error_code(std::errc::timed_out) );
					} );
					it = _req_packet_list.erase( it );
				}
				else
				{
					++it;
				}
			}
		}
		_timer.AddTask( 100ms, std::bind( &UserService::TimerLoop, this ) );
	}

	void UserService::RecvPacket( std::error_code ec, std::shared_ptr<RAUserMessage> pb )
	{

		int16_t sn = pb->sn();
		if(sn > 0)
		{
			std::unique_lock<std::mutex> lock( _sns_mutex );
			auto it = _req_packet_list.find( sn );
			if(it == _req_packet_list.end())
			{
				printf( "超时，被忽略的包。\n" );
				return;
			}
			else
			{
				auto wait = it->second;
				_req_packet_list.erase( it );
				_task->AddTask( [=]{
					wait->cb( pb, std::error_code() );
				} );
				return;
			}
		}

		_RecvPacket( pb );
	}

	void UserService::SendPacket( RAUserMessagePtr pb, tick_t timeout, std::function<void( RAUserMessagePtr, std::error_code )> cb )
	{
		_sns_mutex.lock();
		++_sn;
		if( _sn <= 0)
		{
			_sn = 1;
		}

		pb->set_sn( _sn );
		WaitRespPacketPtr wait = std::make_shared<WaitRespPacket>();
		wait->timeout = timeout;
		wait->cb = std::move(cb);
		wait->pb = pb;
		wait->calltime = TimeStamp();
		_req_packet_list[_sn] = wait;
		_sns_mutex.unlock();
		auto buf = _proto_packet.Build( pb );
		if(buf)
		{
			Write( buf );
		}
	}

	void UserService::SendPacket( RAUserMessagePtr pb )
	{
		pb->set_sn( 0 );
		auto buf = _proto_packet.Build( pb );
		if(buf)
		{
			Write( buf );
		}
	}

}