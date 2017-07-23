#include "user_manager.h"

#include "server_module.h"
#include "socket_manager.h"
#include "server_config.h"

#include "user.h"

UserManager::UserManager()
{
    _socket_mgr = ServerModule::GetInstance()->GetSocketManager();
}

UserManager::~UserManager()
{
    
}

void UserManager::Start()
{
    std::string ip;
    int16_t port;
    if ( ServerModule::GetInstance()->GetServerCnfig()->GetServer(1, ip, port) )
    {
        _socket_mgr->AsyncAccept( ip, port, std::bind( &UserManager::HandleAccept, this, std::placeholders::_1, std::placeholders::_2 ) );
    }
}

void UserManager::Stop()
{
    _stop = true;
    _lock.lock();
    for (auto & u: _users )
    {
        u->DettachTcp();
    }
    _lock.unlock();
}

void UserManager::HandleLogin( std::shared_ptr<User> user, BufferPtr buf)
{
    _lock.lock();
    for ( auto u : _users )
    {
        if ( u->userid() == user->userid() )
        {
            continue;
        }
        u->SendPacket(1,buf);
    }
    _lock.unlock();
}

void UserManager::HandleLogout( std::shared_ptr<User> user, BufferPtr buf )
{
     
}

bool UserManager::HandleAccept( std::error_code ec, socket_t fd )
{
    if (ec == asio::error::basic_errors::bad_descriptor )
    {
        printf("accept error:%s\n",ec.message().c_str());
    }
    if ( !ec)
    {
        std::string ip;
        int16_t port;
        auto sm = ServerModule::GetInstance()->GetSocketManager();
        if ( sm->QuerySocketInfo( fd, ip, port ) )
        {
            printf( "收到客户端新连接：%s:%u\n",ip.c_str(), (uint16_t)port );
        }
        auto user = std::make_shared<User>(this);
        user->AttachTcp( fd );
        _lock.lock();
        _users.push_back(user);
        _lock.unlock();
    }
    if ( _stop )
    {
        return false;
    }
    return true;
}

