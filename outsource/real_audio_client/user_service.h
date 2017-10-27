#include <memory>
#include "base/async_task.h"
#include "protobuf_packet.h"
#include "user_service.pb.h"
#include "base/tcp_socket.h"
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
    void ConnectServer( int server_type, std::string ip, int port );
    void DisconnectServer( int server_type );
	bool IsConnectServer();
    void RegisterHandler( ProtoPacketizer *p );
    void UnRegisterHandler( ProtoPacketizer* p );

    void RecvPacket( int server_type, std::error_code ec, std::shared_ptr<audio_engine::RAUserMessage> pb );
    void SendPacket( int server_type, std::shared_ptr<audio_engine::RAUserMessage> pb );
private:
    void     Read( int server_type,BufferPtr buf );
    void     Write( int server_type, BufferPtr buf );
    void     HandleConnect( int server_type );
    void     HandleError( int server_type, std::error_code ec );
    AsyncTask*    _task = nullptr;
	TcpSocketPtr _tcp_socket;
    ProtoPacket _proto_packet;
    BufferPool* _buffer_pool;
    std::mutex _lock_handle;
    std::list<ProtoPacketizer*> _proto_handlers;
};

typedef std::shared_ptr<UserService> UserServicePtr;