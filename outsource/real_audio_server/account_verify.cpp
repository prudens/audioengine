#include "account_verify.h"
#include "error_code.h"
#define MIN_SUPPORTED_VERSION "20171105"
AccountVerify::AccountVerify()
{

}

AccountVerify::~AccountVerify()
{

}

bool AccountVerify::JoinRoom( UserConnPtr user )
{

}

int Verify( std::string uid, std::string roomkey, std::string sdkversion )
{
	if (uid.empty())
	{
		return ERR_INVALID_USER_ID;
	}
	if (roomkey.empty())
	{
		return ERR_INVALID_ROOM_KEY;
	}
	if (sdkversion < MIN_SUPPORTED_VERSION)
	{
		return ERR_NOT_VERSION_SUPPORTED;
	}

	//这里做一些具体的验证，暂时先不做。
	return ERR_OK;
}

ClientConnMgr::ClientConnMgr()
{

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
		ASSERT( _acceptor );
		_acceptor->AsyncAccept( std::bind( &UserManager::HandleAccept, this, std::placeholders::_1, std::placeholders::_2 ) );
	}
}

void ClientConnMgr::StopListen()
{
	_acceptor->DisAccept();
	_acceptor.reset();
}

bool UserManager::HandleAccept( std::error_code ec, TcpSocketPtr tcp )
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
		auto user = std::make_shared<UserConnection>( this );
		user->AttachTcp( tcp );
	}
	if(_stop)
	{
		return false;
	}
	_acceptor->AsyncAccept( std::bind( &UserManager::HandleAccept, this, std::placeholders::_1, std::placeholders::_2 ) );
	return true;
}
