#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <mutex>
#include <list>
#include <vector>
#include <cassert>
typedef std::string UID;
typedef uint64_t RID;
#define DEFAULT_BUFFER_SIZE (64*1024-4)

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
    Buffer( size_t capacity = DEFAULT_BUFFER_SIZE )
    {
        _data.resize(capacity);
        _write_pos = 0;
        _read_pos = 0;
    }
    void Write( size_t length)
    {
        _write_pos += length;
        if (_write_pos > _data.size())
        {
            _write_pos = _data.size();
        }
    }

    void Read(size_t length)
    {
        _read_pos += length;
        if ( _read_pos > _write_pos )
        {
            _read_pos = _write_pos;
        }
    }

    size_t WriteAvailable(size_t min_write_size = 0)
    {
        size_t cap = _data.size() - _write_pos;
        if ( cap >= min_write_size )
        {
            return cap;
        }

        Shrink();
        cap = _data.size() - _write_pos;
        if ( cap >= min_write_size )
        {
            return cap;
        }
        size_t more = min_write_size - cap;
        _data.resize(_data.size() + more);
        cap = _data.size() - _write_pos;
        assert( cap >= min_write_size );
        return cap;
    }

    size_t ReadAvailable() const
    {
        return _write_pos - _read_pos;
    }

    void Reset( size_t capacity = DEFAULT_BUFFER_SIZE )
    {
        if (_data.size() < capacity)
        {
            _data.resize( capacity );
        }
        _write_pos = 0;
        _read_pos = 0;
    }


    char* WriteData()
    {
        return _data.data() + _write_pos;
    }
    const char* ReadData()
    {
        return _data.data() + _read_pos;
    }
private:
    void Shrink()
    {
        if ( _read_pos > 0 )
        {
            memmove( _data.data(), _data.data() + _read_pos, _write_pos - _read_pos );
            _write_pos -= _read_pos;
            _read_pos = 0;
        }
    }
    std::vector<char> _data;
    size_t _write_pos;
    size_t _read_pos;
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
            bufptr->Reset(capacity);
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