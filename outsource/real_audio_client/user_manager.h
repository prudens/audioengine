#pragma once
#include <list>
#include "real_audio_common.h"
#include "asio.hpp"
#include "socket_manager.h"

typedef std::function<void( UID userid,int login_result )> LoginHandle;
class UserManager
{
public:
    UserManager();
    ~UserManager();
public:
    void SetEventCallback( LoginHandle handle );
    int Login(UID userid);
    int Logout( UID userid );
    int Logining( UID userid, int &login_status );
private:
    void DoLogin(UID userid);

    struct UserInfo
    {
        UID _user_list;
        int  login_status;
    };
    std::list<UserInfo> _user_list;
    LoginHandle _login_handle;
    socket_t _socket_login = 0;

};