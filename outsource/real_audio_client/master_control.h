#pragma once
#include "user_list.h"
#include "user_manager.h"

class IAsyncEventHandler
{
public:
	virtual void RespondLogin(std::string roomkey, std::string uid, int ec) = 0;
	virtual void RespondLogout(std::string roomkey, std::string uid, int ec) = 0;
};
 class MasterControl final : public UserEventHandler
{
public:
	MasterControl();
	~MasterControl();
public:
	void Initialize();
	void Terminate();

protected://UserEventHandler
	virtual void UpdateLoginState(LoginState state);
	virtual void UserEnterRoom(MemberPtr user);
	virtual void UserLeaveRoom(std::string user_id);
	virtual void UpdateUserState(std::string user_id, int state);
	virtual void UpdateUserExtend(std::string user_id, std::string extend);
public:
	void RegisterEventHandler(IAsyncEventHandler* handler);
	void Login(std::string roomkey, std::string uid);
	void Logout();
private:
	MemberList _room_member_list;
	UserManager _user_mgr;
	IAsyncEventHandler* _event_handler;
};

