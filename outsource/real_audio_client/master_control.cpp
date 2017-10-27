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

void MasterControl::UserLeaveRoom(int64_t token)
{
	_room_member_list.Remove(token);
}

void MasterControl::UpdateUserState(int64_t src_token, int64_t dst_token, int state,int ec)
{
	if (_room_member_list.Update(dst_token, state))
	{
		//这里处理其他模块逻辑
		if (src_token == _user_mgr.GetToken())
		{
			//说明是自己操作的，通知上层。
		}
		else
		{

		}
	}
}

void MasterControl::UpdateUserExtend(int64_t token, std::string extend,int ec)
{
	if (_room_member_list.Update(token, extend))
	{
		//这里处理其他模块逻辑
		if (token == _user_mgr.GetToken())
		{
			//说明是自己设置的，通知上层。
		}
		else
		{

		}
	}
}

void MasterControl::UpdateUserList(const std::vector<MemberPtr>& users)
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
