#pragma once
// 专门有一个线程处理Message的解压缩。
#include <string>
#include <list>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include "real_audio_common.h"
#include "task.h"
typedef int socket_t;
typedef std::function<void( std::error_code ec, int server_type )> ErrorHandle;
typedef std::function<void( BufferPtr buf, int server_type )> BufferHandler;
struct PacketHeader;
class SocketManager;
class PacketHandler:Processer
{
public:
    struct Packet :public Buffer
    {
        int server_type;
    };
    using PacketPtr = std::shared_ptr<Packet>;

    explicit PacketHandler( SocketManager* socket_mgr );
    ~PacketHandler();
    void version( int v ) { _version = v; }
    void prototype( int type ) { _prototype = type; }
    void compress( int c ) { _compress = c; }
    int compress() const{ return _compress; }
    int version() const{ return _version; }
    int prototype() const{ return _prototype; }
    size_t header_size()const;

    void Start();
    void Stop();
    void AddServer( int server_type, std::string ip, int port );
    void RemoveServer( int server_type );
    void Pruduce( int server_type, void* packet, size_t length );
    void SetErrorHandler( ErrorHandle handle ) { _ErrorHandle = handle; }
    void SetProduceHandler( BufferHandler handle ) { _ProductHandle = handle; }
    void SetConsumHandler( BufferHandler handle ) { _ConsumHandle = handle; }
    bool Process();
private:
    void Run();
    bool ParseHeader( PacketHeader* header );
    bool AddHeader(void* data, size_t length);
    void Consum( int server_type );
    void HandleError( std::error_code ec, int server_type );
private:

    SocketManager* _socket_mgr;
    std::mutex _lock_sockets;
    std::map<int, socket_t> _sockets;
    std::list<PacketPtr> _buffer_pool;
    std::list<PacketPtr> _read_consum_packets;
    std::list<PacketPtr> _write_product_packets;
    int _version = 1;
    int _prototype = 1;
    int _compress = 0;
    ErrorHandle _ErrorHandle;
    BufferHandler _ProductHandle;
    BufferHandler _ConsumHandle;
};