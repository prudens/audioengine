#include "user_manager.h"
#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "base/time_cvt.hpp"
#include "server_config.h"
#include "client_module.h"
#include "base/timer.h"
#include "user_service.pb.h"
#include "base/log.h"
using namespace audio_engine;
Logger Log;
UserManager::UserManager( std::shared_ptr<UserService> proto_packet )
{
	Log.setLevel(LEVEL_VERBOSE);
    _user_service = proto_packet;
    if (!_user_service)
    {
        _user_service = std::make_shared<UserService>();
        _user_service->RegisterHandler( this );
    }
	_timer = ClientModule::GetInstance()->CreateSTimer();

}

UserManager::~UserManager()
{
    _user_service->UnRegisterHandler( this );
	_timer->Stop();
	_timer.reset();
}

void UserManager::SetEventCallback(UserEventHandler* handle )
{
    _event_handle = handle;
}

int UserManager::Login(std::string roomkey,std::string userid)
{
    _user_id = userid;
	_roomkey = roomkey;
	_target_state = LS_LOGINED;
	Transform(_cur_state);
	_try_login_count = 0;
    return 0;
}

void UserManager::Logout()
{
	_target_state = LS_NONE;
	Transform(_cur_state);
}

int UserManager::GetCurState()
{
	return _cur_state;
}

int UserManager::GetTargetState()
{
	return _target_state;
}

std::string UserManager::GetUserID()
{
	return _user_id;
}

std::string UserManager::GetRoomKey()
{
	return _roomkey;
}

int64_t UserManager::GetToken()
{
	return _token;
}

void UserManager::DoLogout()
{
    auto pb = std::make_shared<audio_engine::RAUserMessage>();
	auto logout_req = pb->mutable_request_logout();
    logout_req->set_token(_token);
    _user_service->SendPacket( 1, pb );
	Transform(LS_LOGOUT);
	_cur_state_time = TimeStampMs();
	_timer->AddTask(1000, [=] 
	{
		if ((_cur_state_time + 1000 < TimeStampMs()) && (_cur_state == LS_LOGOUT))
		{
			Log.w("logout time out");
			Transform(LS_CONNECTED);
		}
	});
}

void UserManager::ConnectServer()
{
	std::string ip;
	int16_t port;
	if (ClientModule::GetInstance()->GetServerCnfig()->GetServer(1, ip, port))
	{
		_user_service->ConnectServer(1, ip, port );
	}
	Transform(LS_CONNECTING);
}

void UserManager::DisConnectServer()
{
	_user_service->DisconnectServer(1);
	Transform(LS_NONE);
}

void UserManager::VerifyAccount()
{
	auto pb = std::make_shared<audio_engine::RAUserMessage>();
	auto login_req = pb->mutable_request_login();
	login_req->set_userid(_user_id);
	login_req->set_extend(_extend);
	login_req->set_devtype(_device_type);
	_user_service->SendPacket(1, pb);
	Transform(LS_VERIFY_ACCOUNT);
}

bool UserManager::RecvPacket( std::shared_ptr<audio_engine::RAUserMessage> pb )
{
    if (pb->has_responed_login())
    {
        auto &login_res = pb->responed_login();
		_error_code = login_res.error_code();
        auto userid = login_res.userid();
        auto token = login_res.token();
		Log.d("收到用户登陆结果:userid:%s,token:%d,ec:%d\n", userid.c_str(), token, _error_code);
        if (_error_code == 0)
        {
            _user_id = userid;
            _token = token;
			//Transform(LS_LOGINED);
        }
		else
		{
			if (++_try_login_count > MAX_TRY_LOGIN)
			{
				_target_state = LS_NONE;
			}
			Transform(LS_CONNECTED);
		}
    }
    if ( pb->has_responed_logout() )
    {
		Log.v( "收到登出消息回复\n" );
        auto logout_res = pb->responed_logout();
		_error_code = logout_res.error_code();
        if (logout_res.token() == _token)
        {
			Log.d("退出操作结果:%d\n", _error_code);
        }
        else
		{
            Log.w("未知token，抛弃当前包\n");
			
        }
		_cur_state_time = TimeStampMs();
		Transform(LS_CONNECTED);
    } 
    if ( pb->has_notify_login() )
    {
        auto login_ntf = pb->notify_login();
		auto userid = login_ntf.userid();
		std::string extend = login_ntf.extend();
		int dev_type = login_ntf.devtype();
		int status = login_ntf.state();
        Log.d( "new user online :\nuserid:%s\nextend:%s\ndev_type:%d\n",
                userid.c_str(), extend.c_str(), dev_type );
		auto user = CreateMember();
		user->SetUserID(userid);
		user->SetUserExtend(login_ntf.extend());
		user->SetToken( login_ntf.token() );
		user->SetUserState(login_ntf.state());
		user->SetDeviceType(dev_type);
		if (_event_handle)
		{
			_event_handle->UserEnterRoom(user);
		}
    }

	if (pb->has_notify_logout())
	{
		auto logout_ntf = pb->notify_logout();
		if (_event_handle)
		{
			_event_handle->UserLeaveRoom(logout_ntf.token());
		}
	}

	if (pb->has_update_user_state())
	{
		auto update_user_state = pb->update_user_state();
		if (_event_handle)
		{
			_event_handle->UpdateUserState(update_user_state.src_token(), update_user_state.dst_token(), update_user_state.state(), update_user_state.error_code());
		}
	}
	if (pb->has_update_user_extend())
	{
		auto update_user_extend = pb->update_user_extend();
		if (_event_handle)
		{
			_event_handle->UpdateUserExtend(update_user_extend.token(), update_user_extend.extend(), update_user_extend.error_code());
		}
	}

	if (pb->has_notify_user_list())
	{
		// 能够走到这里，说明一定登陆验证成功了。
		auto user_list = pb->notify_user_list();
		auto size = user_list.user_size();
		if (user_list.end_flag() & FLAG_FIRST_PKG)
		{
			_cache_userlist.clear();
		}
		for (int i = 0; i < size; i++)
		{
			const audio_engine::UserInfo & u = user_list.user(i);
			auto user = CreateMember();
			user->SetUserID(u.userid());
			user->SetUserExtend(u.extend());
			user->SetToken(u.token());
			user->SetUserState(u.state());
			user->SetDeviceType(u.devtype());
			_cache_userlist.push_back(user);
		}

		if (user_list.end_flag() & FLAG_LAST_PKG)
		{
			if (_event_handle)
			{
				_event_handle->UpdateUserList(_cache_userlist);
			}
			Transform(LS_LOGINED);
		}
	}
    return false;
}

bool UserManager::HandleError( int server_type, std::error_code ec )
{
	if (_user_service->IsConnectServer() || LS_NONE != _cur_state)
	{
		Log.w("socket[%d] 读写失败：%s\n", server_type,ec.message());
		if (++_try_login_count > MAX_TRY_LOGIN)
		{
			_target_state = LS_NONE;
		}
		Transform(LS_NONE);
	}
    return server_type == 1;
}

bool UserManager::HandleConnect( int server_type )
{
    //printf( "连接服务器(%d)成功！！！\n", server_type );
	Log.d("连接服务器(%d)成功.\n", server_type);
	Transform(LS_CONNECTED);
	return server_type == 1;

}

void UserManager::Transform(LoginState state)
{
	Update(state);
	switch (_target_state)
	{
	case LS_NONE:
		switch (state)
		{
		case LS_NONE:
			break;
		case LS_CONNECTING:
			break;
		case LS_CONNECTED:
			DisConnectServer();
			break;
		case LS_VERIFY_ACCOUNT:
			break;
		case LS_LOGOUT:
			break;
		case LS_LOGINED:
			DoLogout();
			break;
		default:
			break;
		}
		break;
	case LS_CONNECTING:
		break;
	case LS_CONNECTED:
		switch (state)
		{
		case LS_NONE:
			ConnectServer();
			break;
		case LS_CONNECTING:
			break;
		case LS_CONNECTED:
			break;
		case LS_VERIFY_ACCOUNT:
			break;
		case LS_LOGOUT:
			break;
		case LS_LOGINED:
			break;
		default:
			break;
		}
		break;
	case LS_VERIFY_ACCOUNT:
		break;
	case LS_LOGOUT:
		break;
	case LS_LOGINED:
		switch (state)
		{
		case LS_NONE:
			ConnectServer();
			break;
		case LS_CONNECTING:
			break;
		case LS_CONNECTED:
			VerifyAccount();
			break;
		case LS_VERIFY_ACCOUNT:
			break;
		case LS_LOGOUT:
			break;
		case LS_LOGINED:
			
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}


}


void UserManager::Update(LoginState state)
{
	if ( _cur_state != state )
	{
		if (_event_handle && (state % 2 == 0))
		{
			_event_handle->UpdateLoginState(state);
		}
	}
	_cur_state = state;
}
