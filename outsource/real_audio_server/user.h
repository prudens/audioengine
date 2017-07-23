#pragma once
#include <memory>

#include "base/async_task.h"
#include "real_audio_common.h"
#include "protobuf_packet.h"
#include "user_service.pb.h"
typedef int socket_t;
class SocketManager;
class UserManager;
class User : public std::enable_shared_from_this<User>
    ,public PacketHandle
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
    virtual void RecvPacket( int server_type, audio_engine::RAUserMessage* buf );
    virtual void SendPacket( int server_type, BufferPtr buf );
private:
    void Send(int type, audio_engine::RAUserMessage* pb );
    void Read(BufferPtr buf);
    void Write(int type, BufferPtr buf );
    void HandleError( std::error_code ec );
    void HandleLogin(const audio_engine::LoginRequest& login_req);
    UID _userid;
    std::string _user_name;
    std::string _extend;
    int64_t _token = 0;
    int _device_type;
    socket_t _sock_id = 0;
    AsyncTask* _task = nullptr;
    SocketManager* _sm = nullptr;
    ProtoPacket _proto_packet;
    UserManager* _host;
    bool _stop = false;
};