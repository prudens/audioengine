#include "user_manager.h"
#include <iostream>
#include <string>
#include <vector>
#include <functional>
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

void UserManager::SetEventCallback( LoginHandle handle )
{
    _login_handle = handle;
}

int UserManager::Login( UID userid )
{
    _user_info.user_id = userid;
	_target_state = LS_LOGINED;
	Transform();
    return 0;
}

void UserManager::Logout()
{
	_target_state = LS_NONE;
	Transform();
}

int UserManager::GetLoginState()
{
	return _cur_state;
}

void UserManager::DoLogout()
{
    auto pb = std::make_shared<audio_engine::RAUserMessage>();
    auto logout_req = pb->mutable_logout_requst();
    logout_req->set_token(_user_info.token);
    _user_service->SendPacket( 1, pb );
	_cur_state = LS_LOGOUT;
	Transform();
}

void UserManager::ConnectServer()
{
	std::string ip;
	int16_t port;
	if (ClientModule::GetInstance()->GetServerCnfig()->GetServer(1, ip, port))
	{
		_user_service->ConnectServer(1, ip, port );
	}
	_cur_state = LS_CONNECTING;
	Transform();
}

void UserManager::DisConnectServer()
{
	_user_service->DisconnectServer(1);
	_cur_state = LS_NONE;
	Transform();
}

void UserManager::VerifyAccount()
{
	auto pb = std::make_shared<audio_engine::RAUserMessage>();
	auto login_req = pb->mutable_login_requst();
	login_req->set_userid(_user_info.user_id);
	login_req->set_username(_user_info.user_id);
	login_req->set_devtype(DEVICE_TYPE::DEVICE_UNKNOWN);
	_user_service->SendPacket(1, pb);

	_cur_state = LS_VERIFY_ACCOUNT;
	Transform();
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
            _user_info.user_id = userid;
            _user_info.token = token;
			_cur_state = LS_LOGINED;
			tUserPtr ptr = std::make_shared<tUser>();
			ptr->device_type = _user_info.device_type;
			ptr->user_id = _user_info.user_id;
			ptr->user_name = _user_info.user_name;
			_user_list.Add(ptr);
        }
		else
		{
			_cur_state = LS_CONNECTED;
		}
		Transform();
		if (_login_handle)
		{
			_login_handle(userid, login_result);
		}

    }
    else if ( pb->has_logout_response() )
    {
        printf( "收到登出消息回复\n" );
        auto logout_res = pb->logout_response();
        if (logout_res.token() == _user_info.token)
        {
			printf("退出操作结果:%d",(int)logout_res.status());
        }
        else
        {
            printf("Unknown user token");
        }
		_user_list.Clear();
		_cur_state = LS_CONNECTED;
		Transform();
    } 
    else if ( pb->has_login_notify() )
    {
        auto login_ntf = pb->login_notify();
        UID userid = login_ntf.userid();
        std::string username = login_ntf.username();
        std::string extend = login_ntf.extend();
        int dev_type = login_ntf.devtype();
        int status = login_ntf.status();
        if (status == 1)
        {
            printf( "new user online :\nuserid:%s\nusername:%s\nextend:%s\ndev_type:%d\n",
                    userid.c_str(), username.c_str(), extend.c_str(), dev_type );
			tUserPtr ptr = std::make_shared<tUser>();
			ptr->device_type = dev_type;
			ptr->user_id = userid;
			ptr->user_name = username;
			_user_list.Add(ptr);
        }
        else
        {
            printf( "new user offline :\nuserid:%s\nusername:%s\n",
                    userid.c_str(), username.c_str() );
			_user_list.Remove(userid);
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
    if ( ec.value() != 0 )
    {
		if (_login_handle)
		{
			_login_handle(_user_info.user_id, 0);
		}
    }
	_cur_state = LS_NONE;
	Transform();
    return true;
}

bool UserManager::HandleConnect( int server_type )
{
    printf( "连接服务器(%d)成功！！！\n", server_type );
	_cur_state = LS_CONNECTED;
	Transform();
    return true;
}


void UserManager::OnTimer()
{
}

void UserManager::Transform()
{
	LoginState state = _cur_state;
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

	Update();
}


void UserManager::Update()
{
	if ( _cur_state < LS_LOGINED )
	{
		_user_list.Clear();
	}
}
