#pragma once
#include <cstdint>
#include <memory>
#include <string>
typedef std::string UID;
typedef uint64_t RID;
#define DEFAULT_BUFFER_SIZE (64*1024)
struct Buffer 
{
    Buffer( char* data, size_t capacity, size_t length )
    {
        this->data = data;
        this->capacity = capacity;
        this->length = length;
    }

    Buffer( size_t capacity = DEFAULT_BUFFER_SIZE )
    {
        data = new char[capacity];
        this->capacity = capacity;
        length = 0;
    }

    ~Buffer()
    {
        if(data)
            delete[] data;
    }

    void Resize( size_t capacity )
    {
        if (capacity <= this->capacity)
        {
            return;
        }
        delete[] data;
        data = new char[capacity];
        this->capacity = capacity;
        this->length = 0;
    }
    char* data;
    size_t capacity;
    size_t length;
};

typedef std::shared_ptr<Buffer> BufferPtr;

#pragma pack(1)
struct PacketHeader
{
    char version : 2;
    char prototype : 3;
    char compress : 3;
    uint32_t content_length;
};
#pragma  pack()