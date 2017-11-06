#include "user.h"
#include "server_module.h"
#include "user_service.pb.h"
#include "token_generater.h"
#include "user_manager.h"
namespace audio_engine{
	UserConnection::UserConnection( UserManager* host )
		: _host( host )
		, _proto_packet( std::bind( &UserConnection::RecvPacket, this, std::placeholders::_1, std::placeholders::_2 ) )
	{
		_buffer_pool = ServerModule::GetInstance()->GetBufferPool();
		_task = new AsyncTask( ServerModule::GetInstance()->GetThreadPool() );
	}

	UserConnection::UserConnection( TcpSocketPtr tcp )
	{

	}

	void UserConnection::SetVerifyAccountCB( std::function<int( RAUserMessagePtr pb, UserConnPtr conn )> cb )
	{
		_VerifyAccount = cb;
	}

	UserConnection::~UserConnection()
	{
		DettachTcp();
		printf( "user:%s offline\n", _userid.c_str() );
	}

	void UserConnection::AttachTcp( TcpSocketPtr tcp )
	{
		_tcp_socket = tcp;
		Read();
	}

	void UserConnection::DettachTcp()
	{
		if(!_tcp_socket)
		{
			return;
		}
		_stop = true;
		_tcp_socket->DisConnect();
	}

	std::string UserConnection::userid()
	{
		return _userid;
	}

	int64_t UserConnection::token()
	{
		return _token;
	}

	std::string UserConnection::extend()
	{
		return _extend;
	}

	audio_engine::DEVICE_TYPE UserConnection::device_type()
	{
		return _device_type;
	}

	void UserConnection::Read()
	{
		if(_stop)
		{
			return;
		}
		if(!_tcp_socket)
		{
			return;
		}
		auto buf = _buffer_pool->PullFromBufferPool();
		auto self = shared_from_this();
		_tcp_socket->AsyncRead( buf->WriteData(), buf->WriteAvailable(),
			[=]( std::error_code ec, std::size_t length )
		{
			if(!ec)
			{
				buf->Write( length );
				self->_task->AddTask( [=]
				{
					self->_proto_packet.Parse( buf );
				} );
			}
			else
			{
				HandleError( ec );
			}

			Read();
		} );
	}

	void UserConnection::Write( int type, BufferPtr buf )
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
				self->HandleError( ec );
			}
		} );
	}

	void UserConnection::RecvPacket( std::error_code ec, std::shared_ptr< audio_engine::RAUserMessage> pb )
	{
		if(!pb)
		{
			return;
		}
		int err;
		if ( _first_packet )
		{
			_first_packet = false;
			auto self = shared_from_this();
			err = _VerifyAccount( pb,self );
		}

		if (err != 0)
		{
			_stop = true;
			VerifyFailed(pb);
			return;
		}

		// 正常流程处理。

	}

	void UserConnection::VerifyFailed(RAUserMessagePtr pb)
	{
	}
	void UserConnection::HandleError( std::error_code ec )
	{
		if(_stop)
		{
			return;
		}

		if(ec)
		{
			_tcp_socket.reset();
			printf( ec.message().c_str() );
			printf( "\n" );
			_stop = true;
			if(_host)
			{
				auto self = shared_from_this();
				_host->HandleLogout( self );
			}
		}

	}

	void UserConnection::HandleLogin( const audio_engine::RequestLogin& login_req,int sn )
	{
		_userid = login_req.userid();
		_extend = login_req.extend();
		_device_type = login_req.devtype();
		auto pb = std::make_shared<audio_engine::RAUserMessage>();
		auto login_res = pb->mutable_responed_login();
		_token = ServerModule::GetInstance()->GetTokenGenerater()->NewToken( _userid );
		login_res->set_token( _token );
		login_res->set_userid( _userid );
		login_res->set_error_code( 0 );
		pb->set_sn( sn );
		auto user_list = pb->mutable_notify_user_list();
		auto users = _host->GetUserList();
		auto size = users.size();
		for(const auto& user : users)
		{
			auto item = user_list->add_user();
			item->set_userid( user->userid() );
			item->set_devtype( user->device_type() );
			item->set_extend( user->extend() );
			item->set_token( user->token() );
			item->set_state( user->state() );
		}

		user_list->set_pkg_flag( audio_engine::FLAG_LAST_PKG | audio_engine::FLAG_FIRST_PKG );
		Write( 0, _proto_packet.Build( pb ) );
		if(_host)
		{
			auto self = shared_from_this();
			_host->HandleLogin( self );
		}
		printf( "用户：%s登陆\n", _userid.c_str() );
	}

	void UserConnection::HandleLogout( const ::audio_engine::RequestLogout& logout_req,int sn )
	{
		auto self = shared_from_this();
		auto token = logout_req.token();
		if(token != _token)
		{
			printf( "无效请求，token不对" );
			return;
		}
		if(!_tcp_socket)
		{
			return;
		}
		auto pb = std::make_shared<audio_engine::RAUserMessage>();
		auto logout_res = pb->mutable_responed_logout();
		logout_res->set_token( _token );
		logout_res->set_error_code( 0 );
		pb->set_sn(sn);
		BufferPtr buf = _proto_packet.Build( pb );
		if(buf)
		{
			_tcp_socket->AsyncWrite( buf->ReadData(), buf->ReadAvailable(),
				[=]( std::error_code ec, std::size_t length )
			{
				self->_buffer_pool->PushToBufferPool( buf );
			} );
		}

		if(_host)
		{
			_host->HandleLogout( self );
		}
	}

	void UserConnection::Send( int type, BufferPtr buf )
	{
		Write( type, buf );
	}
}