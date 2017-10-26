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

using namespace audio_engine;

UserManager::UserManager( std::shared_ptr<UserService> proto_packet )
{
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

void UserManager::DoLogout()
{
    auto pb = std::make_shared<audio_engine::RAUserMessage>();
    auto logout_req = pb->mutable_logout_requst();
    logout_req->set_token(_token);
    _user_service->SendPacket( 1, pb );
	Transform(LS_LOGOUT);
	_cur_state_time = TimeStampMs();
	_timer->AddTask(1000, [=] 
	{
		if (_cur_state_time + 1000 < TimeStampMs())
		{
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
	auto login_req = pb->mutable_login_requst();
	login_req->set_userid(_user_id);
	login_req->set_username(_user_name);
	login_req->set_devtype(DEVICE_TYPE::DEVICE_WINDOWS);
	_user_service->SendPacket(1, pb);
	Transform(LS_VERIFY_ACCOUNT);
}

bool UserManager::RecvPacket( std::shared_ptr<audio_engine::RAUserMessage> pb )
{
    if (pb->has_login_response())
    {
        auto login_res = pb->login_response();
        auto login_result = login_res.result();
        auto userid = login_res.userid();
        auto token = login_res.token();
        std::cout << " username: " << userid << "\n"
            << "login_result: " << login_result << "\n"
            << "token: " << token << "\n";
        if (login_result)
        {
            _user_id = userid;
            _token = token;
			auto user = CreateMember();
			user->SetDeviceType(_device_type);
			user->SetUserID( _user_id);
			user->SetUserName( _user_name);
			user->SetStatus(0);
            if (_event_handle )
            {
				_event_handle->UserEnterRoom(user);
            }
			Transform(LS_LOGINED);
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
    else if ( pb->has_logout_response() )
    {
        printf( "收到登出消息回复\n" );
        auto logout_res = pb->logout_response();
        if (logout_res.token() == _token)
        {
			printf("退出操作结果:%d",(int)logout_res.status());
        }
        else
        {
            printf("Unknown user token");
        }
		Transform(LS_CONNECTED);
    } 
    else if ( pb->has_login_notify() )
    {
        auto login_ntf = pb->login_notify();
        auto userid = login_ntf.userid();
        std::string username = login_ntf.username();
        std::string extend = login_ntf.extend();
        int dev_type = login_ntf.devtype();
        int status = login_ntf.status();
        if (status == 1)
        {
            printf( "new user online :\nuserid:%s\nusername:%s\nextend:%s\ndev_type:%d\n",
                    userid.c_str(), username.c_str(), extend.c_str(), dev_type );
			auto user = CreateMember();
			user->SetDeviceType( dev_type);
			user->SetUserID(userid);
			user->SetUserName( username );
			if (_event_handle)
			{
				_event_handle->UserEnterRoom(user);
			}
        }
        else
        {
            printf( "new user offline :\nuserid:%s\nusername:%s\n",
                    userid.c_str(), username.c_str() );
           if ( _event_handle )
           {
			   _event_handle->UserLeaveRoom(userid);
           }
        }
    }
    else
    {
        printf( "收到不识别的buffer\n" );
        return false;
    }
    return true;
}

bool UserManager::HandleError( int server_type, std::error_code ec )
{
    std::cout << "server_type:"<<server_type<<" "<< ec.message() << "\n";
	if (++_try_login_count > MAX_TRY_LOGIN)
	{
		_target_state = LS_NONE;
	}
	Transform(LS_NONE);

    return true;
}

bool UserManager::HandleConnect( int server_type )
{
    printf( "连接服务器(%d)成功！！！\n", server_type );
	Transform(LS_CONNECTED);
    return true;
}


void UserManager::OnTimer()
{

}

void UserManager::Transform(LoginState state)
{
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
			VerifyAccount();
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

	Update(state);
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
