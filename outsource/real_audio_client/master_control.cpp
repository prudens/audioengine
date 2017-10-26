#include "master_control.h"

MasterControl::MasterControl()
{
}


MasterControl::~MasterControl()
{
}

void MasterControl::Initialize()
{
	_user_mgr.SetEventCallback(this);
}

void MasterControl::Terminate()
{
	_user_mgr.SetEventCallback(nullptr);
}

void MasterControl::UpdateLoginState(LoginState state)
{
	// 只上报三种情况，登陆成功和失败，登出成功。但是内部要打印具体的操作
	if (state == LS_LOGINED && _user_mgr.GetTargetState() == LS_LOGINED)
	{
		_event_handler->RespondLogin(_user_mgr.GetRoomKey().c_str(), _user_mgr.GetUserID().c_str(),0);
	}
	else if (state == LS_NONE && _user_mgr.GetTargetState() == LS_NONE)
	{
		_event_handler->RespondLogout(_user_mgr.GetRoomKey().c_str(), _user_mgr.GetUserID().c_str(), 0);
	}
	else if (_user_mgr.GetTargetState() == LS_LOGINED && state == LS_RESET)
	{
		_event_handler->RespondLogin(_user_mgr.GetRoomKey().c_str(), _user_mgr.GetUserID().c_str(), -2);
	}
}

void MasterControl::UserEnterRoom(MemberPtr user)
{
	_room_member_list.Add(user);
}

void MasterControl::UserLeaveRoom(std::string user_id)
{
	_room_member_list.Remove(user_id);
}

void MasterControl::UpdateUserState(std::string user_id, int state)
{

}

void MasterControl::UpdateUserExtend(std::string user_id, std::string extend)
{
}

void MasterControl::RegisterEventHandler(IAsyncEventHandler * handler)
{
	_event_handler = handler;
}

void MasterControl::Login(std::string roomkey, std::string uid)
{
	_user_mgr.Login(roomkey,uid);
}

void MasterControl::Logout()
{
	_user_mgr.Logout();
}
