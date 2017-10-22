#pragma once
#include <list>
#include "real_audio_common.h"
#include "protobuf_packet.h"
#include "user_service.h"
typedef std::function<void( UID userid,int login_result )> LoginHandle;

struct UserInfo
{
    enum {
        USERSTATUS_NONE,
        USERSTATUS_LOGINING,
        USERSTATUS_LOGINED,
        USERSTATUS_LOGOUT,
        USERSTATUS_LOGINFAILED,
    };
    UID  user_id;
    std::string user_name;
    std::string extend;
    int  login_status;
    int64_t  token;
};

class UserManager: ProtoPacketizer
{
public:
    UserManager( std::shared_ptr<UserService>  proto_packet = nullptr );
    ~UserManager();
public:
    void ConnectServer();
    void SetEventCallback( LoginHandle handle );
    int  Login(UID userid);
    void Logout();
    int  Logining();
private:
    void DoLogin();
    void DoConectServer();
    void DisConectServer();
public:
    virtual bool RecvPacket( std::shared_ptr<audio_engine::RAUserMessage> pb );
    virtual bool HandleError( int server_type, std::error_code ec );
    virtual bool HandleConnect( int server_type );
    LoginHandle _login_handle;
    std::shared_ptr<UserService> _user_service;
    UserInfo _my_user_info;
    bool _connect_server = false;
};