#include "user_service.h"

#include "client_module.h"
#include "base/async_task.h"
#include "base/common_defines.h"
namespace audio_engine{
	UserService::UserService()
		:_proto_packet( std::bind( &UserService::RecvPacket, this, std::placeholders::_1, std::placeholders::_2 ) )
	{
		_buffer_pool = ClientModule::GetInstance()->GetBufferPool();
		_task = ClientModule::GetInstance()->GetAsyncTask();
		_timer = ClientModule::GetInstance()->CreateSTimer();
	}

	UserService::~UserService()
	{

	}

	void UserService::ConnectServer( std::string ip, int port )
	{
		_sns_mutex.lock();
		_sns.clear();
		_sns_mutex.unlock();
		auto self = shared_from_this();
		TcpFactory* fac = ClientModule::GetInstance()->GetTcpFactory();
		_tcp_socket = fac->CreateTcpConnection( "", 0 );
		ASSERT( _tcp_socket );
		_tcp_socket->AsyncConnect( ip, port, [=]( std::error_code ec )
		{
			if(!ec)
			{
				BufferPtr buf = _buffer_pool->PullFromBufferPool();
				self->Read( buf );
				_task->AddTask( [=]
				{
					self->HandleConnect();
				} );

			}
			else
			{
				self->HandleError( ec );
			}
		} );
	}

	void UserService::DisconnectServer()
	{
		if(_tcp_socket)
		{
			_tcp_socket->DisConnect();
			_tcp_socket.reset();
			_sns_mutex.lock();
			_sns.clear();
			_sns_mutex.unlock();
		}

	}

	bool UserService::IsConnectServer()
	{
		return _tcp_socket != nullptr;
	}

	void UserService::RegisterHandler( ProtoPacketizer *p )
	{
		_lock_handle.lock();
		_proto_handlers.push_back( p );
		_lock_handle.unlock();
	}

	void UserService::UnRegisterHandler( ProtoPacketizer* p )
	{
		_lock_handle.lock();
		_proto_handlers.remove( p );
		_lock_handle.unlock();
	}

	bool UserService::RemoveSn( int16_t sn )
	{
		auto it = std::find( _sns.begin(), _sns.end(), sn );
		if(it == _sns.end())
		{
			return false;
		}
		_sns.erase( it );
		return true;
	}

	void UserService::Read( BufferPtr buf )
	{
		if(!_tcp_socket)
		{
			return;
		}
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
				auto buf = _buffer_pool->PullFromBufferPool();
				Read( buf );
			}
			else
			{
				HandleError( ec );
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
			self->_buffer_pool->PushToBufferPool( buf );
			if(ec)
			{
				self->HandleError( ec );
			}
		} );
	}


	void UserService::HandleError( std::error_code ec )
	{
		_lock_handle.lock();
		for(auto&p : _proto_handlers)
		{
			if(p->HandleError( ec ))
			{
				break;
			}
		}
		_lock_handle.unlock();
	}

	void UserService::HandleConnect()
	{
		_lock_handle.lock();
		for(auto& p : _proto_handlers)
		{
			if(p->HandleConnect())
			{
				break;
			}
		}
		_lock_handle.unlock();

	}

	void UserService::RecvPacket( std::error_code ec, std::shared_ptr<RAUserMessage> pb )
	{
		_sns_mutex.lock();
		int16_t sn = pb->sn();
		if(sn > 0)
		{
			auto it = std::find( _sns.begin(), _sns.end(), sn );
			if(it == _sns.end())
			{
				printf( "超时被忽略的包。\n" );
				return;
			}
			else
			{
				_sns.erase( it );
			}
		}
		_sns_mutex.unlock();
		_lock_handle.lock();
		for(auto& p : _proto_handlers)
		{
			if(p->RecvPacket( pb ))
			{
				break;
			}
		}
		_lock_handle.unlock();
	}

	int16_t UserService::SendPacket( std::shared_ptr<RAUserMessage> pb )
	{
		int sn = 0;
		_sns_mutex.lock();
		if(++_sn < 0)
		{
			_sn = 1;
		}
		_sns.push_back( _sn );
		_sn = _sn;
		_sns_mutex.unlock();
		pb->set_sn( _sn );
		auto buf = _proto_packet.Build( pb );
		if(buf)
		{
			Write( buf );
		}

		return _sn;
	}
}