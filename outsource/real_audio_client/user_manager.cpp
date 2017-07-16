#include "user_manager.h"
#include "server_config.h"
#include "socket_manager.h"
#include "client_module.h"
UserManager::UserManager()
{

}

UserManager::~UserManager()
{

}

void UserManager::SetEventCallback( LoginHandle handle )
{
    _login_handle = handle;
}

int UserManager::Login( UID userid )
{
    return 0;
}

int UserManager::Logout( UID userid )
{
    return -1;
}

int UserManager::Logining( UID userid, int &login_status )
{
    return -1;
}

void UserManager::DoLogin(UID userid)
{
    if ( _socket_login  == 0)
    {
        return;
    }
}