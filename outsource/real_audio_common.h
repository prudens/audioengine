#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <mutex>
#include <list>
typedef std::string UID;
typedef uint64_t RID;
#define DEFAULT_BUFFER_SIZE (64*1024-4)
#define RAW_HEADER_SIZE 4
inline void IntToChar(int32_t v, char*c)
{
    c[0] = v >> 24;
    c[1] = v >> 16;
    c[2] = v >> 8;
    c[3] = v & 0xff;
}
inline void CharToInt( const char*c, int32_t& v )
{
    v = (c[0] << 24) + (c[1] << 16) + (c[2] << 8) + c[3];
}

class Buffer 
{
public:
    Buffer( char* data, size_t capacity, size_t length )
    {
        this->_data = data;
        this->_capacity = capacity;
        this->_length = length;
    }

    Buffer( size_t capacity = DEFAULT_BUFFER_SIZE )
    {
        _data = new char[capacity + RAW_HEADER_SIZE];
        memset( _data, 0, RAW_HEADER_SIZE );
        _capacity = capacity;
        _length = 0;
    }

    ~Buffer()
    {
        if(_data)
            delete[] _data;
    }

    void Resize( size_t capacity )
    {
        if (capacity <= _capacity)
        {
            return;
        }
        delete[] _data;
        _data = new char[capacity+RAW_HEADER_SIZE];
        memset( _data, 0, RAW_HEADER_SIZE );
        _capacity = capacity;
        _length = 0;
    }
    char* data()const
    {
        return _data + RAW_HEADER_SIZE;
    }
    size_t length()
    {
        return _length;
    }
    void length( size_t length )
    {
        if ( length != _length)
        {
            IntToChar((int32_t)length, _data);
        }
        _length = length;
    }
    size_t capactiy()
    {
        return _capacity;
    }
    void Release()
    {
        _data = nullptr;
        _capacity = 0;
        _length = 0;
    }
    char*RawData()const
    {
        return _data;
    }
    size_t RawLength()const
    {
        return _length + RAW_HEADER_SIZE;
    }
    size_t RawCapacity()
    {
        return _capacity + RAW_HEADER_SIZE;
    }
private:
    char* _data;
    size_t _capacity;
    size_t _length;
};

typedef std::shared_ptr<Buffer> BufferPtr;

class BufferPool
{
public:
    BufferPtr  PullFromBufferPool( size_t capacity = 64 * 1024 )
    {
        BufferPtr bufptr;
        _lock.lock();
        if ( _buffer_pool.empty() )
        {
            bufptr = std::make_shared<Buffer>( capacity );
        }
        else
        {
            bufptr = _buffer_pool.back();
            bufptr->Resize( capacity );
            _buffer_pool.pop_back();
        }
        _lock.unlock();
        return bufptr;
    }
    void PushToBufferPool( BufferPtr ptr   )
    {
        _lock.lock();
        _buffer_pool.push_back( ptr );
        _lock.unlock();
    }
private:
    std::mutex _lock;
    std::list<BufferPtr> _buffer_pool;
};

#pragma pack(1)
struct PacketHeader
{
    char version : 3;
    char prototype : 3;
    char compress : 2;
    uint32_t content_length;
};
#pragma  pack()