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
	virtual void UserLeaveRoom(int64_t token);
	virtual void UpdateUserState(int64_t src_token, int64_t dst_token, int state,int ec);
	virtual void UpdateUserExtend(int64_t token, std::string extend,int ec);
	virtual void UpdateUserList(const std::vector<MemberPtr>&users);
public:
	void RegisterEventHandler(IAsyncEventHandler* handler);
	void Login(std::string roomkey, std::string uid);
	void Logout();
private:
	MemberList _room_member_list;
	UserManager _user_mgr;
	IAsyncEventHandler* _event_handler;
};

