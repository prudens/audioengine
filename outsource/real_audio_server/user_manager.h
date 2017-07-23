#pragma once
#include <system_error>
#include <list>
#include <memory>
#include "user.h"
#include "user_service.pb.h"

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
    void HandleLogin( std::shared_ptr<User> user, BufferPtr buf );
    void HandleLogout( std::shared_ptr<User> user, BufferPtr buf );
private:
    bool HandleAccept( std::error_code ec, socket_t socket_id );
    SocketManager* _socket_mgr = nullptr;
    bool _stop = false;
    std::mutex _lock;
    std::list<std::shared_ptr<User>> _users;
};