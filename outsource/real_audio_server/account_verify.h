#pragma once
#include <string>
#include "user.h"
class AccountVerify
{
public:
	AccountVerify();
	~AccountVerify();
	int Verify(std::string uid, std::string roomkey,std::string sdkversion);
	bool JoinRoom(UserConnPtr user);
public:

};

class ClientConnMgr
{
public:
	ClientConnMgr();
	~ClientConnMgr();
	void StartListen();
	void StopListen();
private:
	TcpAcceptorPtr _acceptor;
};