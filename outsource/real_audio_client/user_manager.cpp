#include "user_manager.h"
#include <iostream>
#include <string>
#include "server_config.h"
#include "socket_manager.h"
#include "client_module.h"
using namespace audio_engine;

UserManager::UserManager( std::shared_ptr<UserService> proto_packet )
{
    _user_service = proto_packet;
    if (!_user_service)
    {
        _user_service = std::make_shared<UserService>();
        _user_service->RegisterHandler( this );
    }
}

UserManager::~UserManager()
{
    DisConectServer();
    _user_service->UnRegisterHandler( this );

}

void UserManager::SetEventCallback( LoginHandle handle )
{
    _login_handle = handle;
}

int UserManager::Login( UID userid )
{
    _my_user_info.user_id = userid;
    _my_user_info.login_status = UserInfo::USERSTATUS_LOGINING;
    if (_connect_server)
    {
        DoLogin();
    }

    return 0;
}

void UserManager::Logout()
{
    auto pb = _user_service->AllocProtoBuf();
    auto logout_req = pb->mutable_logout_requst();
    logout_req->set_user_token(_my_user_info.token);
    _user_service->Produce(1, &pb, sizeof( pb ) );
}

int UserManager::Logining( )
{
    return _my_user_info.login_status;
}

void UserManager::DoLogin()
{
    if (_my_user_info.login_status == UserInfo::USERSTATUS_LOGINING)
    {
        audio_engine::RAUserMessage* pb = _user_service->AllocProtoBuf();
        auto login_req = pb->mutable_login_requst();
        login_req->set_userid( _my_user_info.user_id );
        login_req->set_username( _my_user_info.user_id );
        login_req->set_type( DEVICE_TYPE::DEVICE_UNKNOWN );
        _user_service->Produce( 1, &pb, sizeof( pb ) );
    }

}

bool UserManager::RecvPacket( audio_engine::RAUserMessage* pb )
{
    if (pb->has_login_response())
    {
        auto login_res = pb->login_response();
        auto login_result = login_res.login_result();
        auto userid = login_res.userid();
        auto token = login_res.user_token();
        std::cout << " username: " << userid << "\n"
            << "login_result: " << login_result << "\n"
            << "token: " << token << "\n";
        if (login_result)
        {
            _my_user_info.user_id = userid;
            _my_user_info.login_status = login_result == 1 ? UserInfo::USERSTATUS_LOGINED : UserInfo::USERSTATUS_NONE;
            _my_user_info.token = token;
        }
        if ( _login_handle )
            _login_handle( userid, login_result );
    }
    else if ( pb->has_logout_response())
    {
        auto logout_res = pb->logout_response();
        if (logout_res.user_token() == _my_user_info.token)
        {
            if ( logout_res.logout_status() == UserInfo::USERSTATUS_LOGOUT)
            {
                _my_user_info.login_status = UserInfo::USERSTATUS_LOGOUT;
            }
            else
            {
                _my_user_info.login_status = UserInfo::USERSTATUS_NONE;
            }
        }
        else
        {
            printf("Unknown user token");
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool UserManager::HandleError( int server_type, std::error_code ec )
{
    std::cout << "server_type:"<<server_type<<" "<< ec.message() << "\n";
    if ( ec == asio::error::basic_errors::connection_refused )
    {
        if ( _my_user_info.login_status == UserInfo::USERSTATUS_LOGINING )
        {
            if ( _login_handle )
            {
                _login_handle( _my_user_info.user_id, UserInfo::USERSTATUS_LOGINFAILED );
            }
        }
    }
    _connect_server = false;
    //DoConectServer();
    return true;
}

bool UserManager::HandleConnect( int server_type )
{
    _connect_server = true;
    DoLogin();
    printf( "连接服务器(%d)成功！！！\n", server_type );
    return true;
}

void UserManager::DoConectServer()
{
    std::string ip;
    int16_t port;
    auto server_cfg = ClientModule::GetInstance()->GetServerCnfig();
    if ( server_cfg->GetServer( 1, ip, port ) )
    {
        _user_service->AddServer( 1, ip, port );
    }
}

void UserManager::DisConectServer()
{
    _user_service->RemoveServer(1);
}

void UserManager::ConnectServer()
{
    DoConectServer();
}
