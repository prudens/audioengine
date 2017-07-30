#pragma once
#include <memory>

#include "base/async_task.h"
#include "real_audio_common.h"
#include "protobuf_packet.h"
#include "user_service.pb.h"


#define DEFAULT_SEND
typedef int socket_t;
class SocketManager;
class UserManager;
class User : public std::enable_shared_from_this<User>
{
public:
    User( UserManager* host);
    ~User();
    void AttachTcp( socket_t fd );
    void DettachTcp();
    UID userid();
    std::string username();
    std::string extend();
    int device_type();
public:
    void RecvPacket(std::shared_ptr< audio_engine::RAUserMessage> pb );
    void Send(int type, BufferPtr buf);
private:
    void Read(BufferPtr buf);
    void Write(int type, BufferPtr buf );
    void HandleError( std::error_code ec );
    void HandleLogin(const audio_engine::LoginRequest& login_req);
    void HandleLogout( const ::audio_engine::LogoutRequst& logout_req);
    UID _userid;
    std::string _user_name;
    std::string _extend;
    int64_t _token = 0;
    int _device_type = 0;
    socket_t _sock_id = 0;

    BufferPool* _buffer_pool = nullptr;
    AsyncTask* _task = nullptr;
    SocketManager* _sm = nullptr;
    ProtoPacket _proto_packet;
    UserManager* _host;

    bool _stop = false;
};