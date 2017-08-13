#include <memory>
#include "base/async_task.h"
#include "socket_manager.h"
#include "protobuf_packet.h"
#include "user_service.pb.h"
class ProtoPacketizer
{
public:
    virtual bool RecvPacket( std::shared_ptr<audio_engine::RAUserMessage> pb ) = 0;
    virtual bool HandleError( int server_type, std::error_code ec ) = 0;
    virtual bool HandleConnect( int server_type ) = 0;
};

class UserService :
    public std::enable_shared_from_this<UserService>
{
public:
    UserService();
    ~UserService();
    void AddServer( int server_type, std::string ip, int port );
    void RemoveServer( int server_type );

    void RegisterHandler( ProtoPacketizer *p );
    void UnRegisterHandler( ProtoPacketizer* p );

    void RecvPacket( int server_type, std::error_code ec, std::shared_ptr<audio_engine::RAUserMessage> pb );
    void SendPacket( int server_type, std::shared_ptr<audio_engine::RAUserMessage> pb );
private:
    socket_t GetSocket( int server_type );
    void     SetSocket( int server_type, socket_t fd );
    void     Read( int server_type,BufferPtr buf );
    void     Write( int server_type, BufferPtr buf );
    void     HandleConnect( int server_type );
    void     HandleError( int server_type, std::error_code ec );
    AsyncTask*    _task = nullptr;
    TcpSocketManager* _socket_mgr = nullptr;
    ProtoPacket _proto_packet;
    BufferPool* _buffer_pool;
    std::mutex _lock_sockets;
    std::map<int, socket_t> _sockets;

    std::mutex _lock_handle;
    std::list<ProtoPacketizer*> _proto_handlers;
};