#pragma once
#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

typedef  std::function<void() > TaskExecute;
struct TimerTask;
class Timer
{
public:
    Timer( int sleepMs = 100);
    ~Timer();
    void AddTask( int elapsed_ms, TaskExecute executer );
    void ClearTask();
private:
    std::thread _worker;
    std::priority_queue<TimerTask> _tasks;
    std::mutex _queue_mutex;
    std::condition_variable condition;
    bool _stop;
};