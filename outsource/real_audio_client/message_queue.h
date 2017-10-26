#pragma once
#include <mutex>
#include <list>
#include <cstdint>
#include <memory>
#include "cmd.h"


class MessageQueue
{
public:
    MessageQueue()=default;
    ~MessageQueue()=default;
    void AddMessage(CMD cmd);
    void AddMessage( stMSG* msg );
    bool GetMessage( stMSG* msg );
    void ClearMessage();
private:
    std::mutex     _mutex;
    std::list< stMSG* > _queue;
};