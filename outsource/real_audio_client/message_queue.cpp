#include "message_queue.h"
#include <chrono>
uint64_t now()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        system_clock::now() - time_point<system_clock>() ).count();
}

void MessageQueue::AddMessage( CMD cmd )
{
    std::lock_guard<std::mutex> lg( _mutex );
    stMSG* msg = new stMSG;
    msg->cmd = cmd;
    _queue.push_back( msg );
}

void MessageQueue::AddMessage( stMSG* msg )
{
    std::lock_guard<std::mutex> lg( _mutex );
    _queue.push_back( msg );
}

bool MessageQueue::GetMessage( stMSG* msg )
{
    std::lock_guard<std::mutex> lg( _mutex );
    if (!_queue.empty())
    {
        auto p = _queue.front();
        *msg = *p;
        delete p;
        _queue.pop_front();
        return true;
    }
    return false;
}

void MessageQueue::ClearMessage()
{
    std::lock_guard<std::mutex> lg( _mutex );
    _queue.clear();
}
