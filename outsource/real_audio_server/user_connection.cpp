#include "user_connection.h"
#include "server_module.h"
#include "user_service.pb.h"
#include "token_generater.h"
namespace audio_engine{
	UserConnection::UserConnection( TcpSocketPtr tcp, TaskThread&thread )
		: _proto_packet(std::bind(&UserConnection::RecvPacket,this,std::placeholders::_1, std::placeholders::_2) )
		, _task( thread )
	{
		_tcp_socket = tcp;
	}

	void UserConnection::SetVerifyAccountCB( std::function<int( RAUserMessagePtr pb, UserConnPtr conn )> cb )
	{
		_VerifyAccount = cb;
	}

	void UserConnection::SetPacketHandler( IPacketHandler * handler )
	{
		_packet_handler = handler;
	}

	UserConnection::~UserConnection()
	{
		DettachTcp();
	}

	void UserConnection::Start()
	{
		if(!_tcp_socket)
		{
			return;
		}
		_stop = false;
		Read();
	}
	void UserConnection::DettachTcp()
	{
		if(!_tcp_socket)
		{
			return;
		}
		_stop = true;
		_packet_handler = nullptr;
		_tcp_socket->DisConnect();//仅仅关闭接收端，发送端不断开，直到发送完成。
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
		auto buf = std::make_shared<Buffer>();
		auto self = shared_from_this();
		_tcp_socket->AsyncRead( buf->WriteData(), buf->WriteAvailable(),
			[=]( std::error_code ec, std::size_t length )
		{
			if(!ec)
			{
				buf->Write( length );
				self->_task.AddTask( [=]
				{
					self->_proto_packet.Parse( buf );
				} );
				Read();
			}
			else
			{
				self->_task.AddTask( [=]
				{
					HandleError( ec );
				} );
			}
		} );
	}

	void UserConnection::Write( BufferPtr buf )
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

	void UserConnection::RecvPacket( std::error_code ec,RAUserMessagePtr pb )
	{
		if(!pb)
		{
			return;
		}
		if(_stop)
		{
			return;
		}
		int err;
		if ( _first_packet )
		{
			_first_packet = false;
			auto self = shared_from_this();
			err = _VerifyAccount( pb,self );
			if(err != 0)
			{
				_stop = true;
				VerifyFailed( pb, err );
				return;
			}
		}



		// 正常流程处理。
		if(_packet_handler)
		{
			_packet_handler->HandlePacket( pb );
		}
	}

	void UserConnection::VerifyFailed( RAUserMessagePtr pb,int ec )
	{
		auto res_login = pb->mutable_responed_login();
		res_login->set_error_code(ec);
		res_login->set_userid( pb->request_login().userid() );
		pb->clear_request_login();
		SendPacket( pb );
	}

	void UserConnection::HandleError( std::error_code ec )
	{
		if(_packet_handler)
		{
			_packet_handler->HandleError( ec );
		}
	}

	void UserConnection::Send( BufferPtr buf )
	{
		Write( buf );
	}

	void UserConnection::SendPacket( RAUserMessagePtr pb )
	{
		auto buf = _proto_packet.Build(pb);
		Write( buf );
	}
}