#pragma once
#include <system_error>
#include <list>
#include <memory>
#include "user.h"
#include "user_service.pb.h"

typedef int socket_t;
class SocketManager;
class UserManager:public std::enable_shared_from_this<UserManager>
{
public:
    UserManager();
    ~UserManager();
public:
    void Start();
    void Stop();
    void HandleLogin( std::shared_ptr<User> user);
    void HandleLogout( std::shared_ptr<User> user );
private:
    bool HandleAccept( std::error_code ec, socket_t socket_id );
    SocketManager* _socket_mgr = nullptr;
    AsyncTask* _task = nullptr;
    ProtoPacket _packet;
    std::mutex _lock;
    std::list<std::shared_ptr<User>> _users;
    bool _stop = false;



};