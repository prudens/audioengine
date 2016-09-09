#pragma once
#ifndef ASYN_TASK_H
#define ASYN_TASK_H
#include <limits>
#include <chrono>
#include <memory>
#include <functional>



using namespace  std::chrono;
using namespace std::placeholders;
class TaskQueue;
typedef  std::function<void( uint32_t ) > TaskExecute;
class AsynTask
{
public:
    AsynTask();
    ~AsynTask();
    uint32_t PostTask( int elapsed_ms, bool repeat, TaskExecute execute );
    void RemoveTask( uint32_t task_id );
    void RemoveAllTask();
private:
    std::shared_ptr<TaskQueue> _task_queue;
    static uint32_t s_task_id;
};

#endif // !ASYN_TASK_H