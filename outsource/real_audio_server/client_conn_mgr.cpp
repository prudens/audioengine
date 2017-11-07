#include "client_conn_mgr.h"
#include "error_code.h"
#include "server_config.h"
#include "server_module.h"
#include "master_control.h"
#define MIN_SUPPORTED_VERSION "20171105"

namespace audio_engine{
	ClientConnMgr::ClientConnMgr(MasterControl *ctrl)
		:_task(ServerModule::GetInstance()->GetThreadPool())
	{
		_master_ctrl = ctrl;
	}

	ClientConnMgr::~ClientConnMgr()
	{

	}

	void ClientConnMgr::StartListen()
	{
		std::string ip;
		int16_t port;
		if(ServerModule::GetInstance()->GetServerCnfig()->GetServer( 1, ip, port ))
		{
			TcpFactory *f = ServerModule::GetInstance()->GetSocketManager();
			_acceptor = f->CreateTcpAcceptr( ip, port );
			//ASSERT( _acceptor );
			_acceptor->AsyncAccept( std::bind( &ClientConnMgr::HandleAccept, this, std::placeholders::_1, std::placeholders::_2 ) );
		}
	}

	void ClientConnMgr::StopListen()
	{
		_acceptor->DisAccept();
		_acceptor.reset();
	}

	bool ClientConnMgr::HandleAccept( std::error_code ec, TcpSocketPtr tcp )
	{
		if(ec)
		{
			printf( "accept error:%s\n", ec.message().c_str() );
		}
		if(!ec)
		{
			std::string ip;
			int16_t port;
			if(!tcp->QuerySocketInfo( ip, port ))
			{
				printf( "收到客户端新连接：%s:%u\n", ip.c_str(), (uint16_t)port );
			}

			auto user_conn = std::make_shared<UserConnection>( tcp, _task.TaskThread() );//保证跟上面同一线程
			user_conn->SetVerifyAccountCB( std::bind( &ClientConnMgr::VerifyAccount, this, std::placeholders::_1,std::placeholders::_2) );
			user_conn->Start();
		}
		if(_stop)
		{
			return false;
		}
		_acceptor->AsyncAccept( std::bind( &ClientConnMgr::HandleAccept, this, std::placeholders::_1, std::placeholders::_2 ) );
		return true;
	}

	int ClientConnMgr::VerifyAccount( RAUserMessagePtr pb, UserConnPtr conn )
	{
		if( pb->has_request_login() )
		{
			auto reqLogin = pb->request_login();

			if(reqLogin.version() < MIN_SUPPORTED_VERSION)
			{
				return ERR_NOT_VERSION_SUPPORTED;
			}
			if(reqLogin.userid().empty())
			{
				return ERR_INVALID_USER_ID;
			}
			if(reqLogin.roomkey().empty())
			{
				return ERR_INVALID_ROOM_KEY;
			}

			_master_ctrl->JoinRoom( pb, conn);
		}
		else
		{
			return ERR_INVALID_ARGUMENT;
		}
		return ERR_OK;
	}
}