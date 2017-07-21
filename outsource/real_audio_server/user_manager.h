#pragma once
#include <system_error>
#include <list>
#include <memory>
#include "user.h"
typedef int socket_t;
class SocketManager;
class UserManager
{
public:
    UserManager();
    ~UserManager();
public:
    void Start();
    void Stop();
private:
    bool HandleAccept( std::error_code ec, socket_t socket_id );
    SocketManager* _socket_mgr = nullptr;
    bool _stop = false;
    std::list<std::shared_ptr<User>> _users;
};