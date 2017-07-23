#pragma once
// 专门有一个线程处理Message的解压缩。
#include <list>
#include <mutex>
#include "real_audio_common.h"

class Packet
{
public:
    explicit Packet();
    virtual ~Packet();
    void version( int v ) { _version = v; }
    void prototype( int type ) { _prototype = type; }
    void compress( int c ) { _compress = c; }
    int compress() const{ return _compress; }
    int version() const{ return _version; }
    int prototype() const{ return _prototype; }
    size_t header_size()const { return sizeof( PacketHeader ); }
    size_t content_length( const void* data )const;
public:
    void Build( int server_type, BufferPtr buf );
    void Parse( int server_type, BufferPtr buf );
    void      PushToBufferPool( BufferPtr ptr );
    BufferPtr PullFromBufferPool( size_t capacity = 64*1024 );
protected:
    virtual void Read( int server_type, void* data, size_t length );
    virtual void Write( int server_type, BufferPtr buf );
    virtual BufferPtr Pack( int server_type, BufferPtr buf );
    virtual BufferPtr UnPack( int server_type, BufferPtr buf );
    bool ParseHeader( PacketHeader* header );
    void AddHeader( BufferPtr buf );
private:
    std::mutex _lock_sockets;
    std::list<BufferPtr> _buffer_pool;
    int _version = 1;
    int _prototype = 1;
    int _compress = 0;
};