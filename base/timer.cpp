#include "timer.h"
typedef std::chrono::milliseconds ms;
typedef std::chrono::seconds      sec;
typedef ms::rep tick_t;
tick_t CurrentTimeMs()
{
    using namespace std::chrono;
    return duration_cast<ms>(
        system_clock::now() - time_point<system_clock>() ).count();
}


struct TimerTask
{
public:
    explicit TimerTask( tick_t elapsed_ms, TaskExecute executer )
    {
        this->elapsed_ms = elapsed_ms;
        this->executer   = std::move(executer);
    }
    TimerTask( TimerTask&& other )
    {
        this->elapsed_ms = other.elapsed_ms;
        this->executer = other.executer;
        other.executer = nullptr;
    }

    TimerTask& operator= ( TimerTask&& other )
    {
        this->elapsed_ms = other.elapsed_ms;
        this->executer = other.executer;
        other.executer = nullptr;
        return *this;
    }

    bool operator < ( const TimerTask& other )const
    {
        return this->elapsed_ms > other.elapsed_ms;
    }
    TimerTask( const TimerTask& other ) = delete;
    TimerTask& operator= ( const TimerTask& other ) = delete;


    tick_t elapsed_ms = 0;
    TaskExecute executer;
};

Timer::Timer(int sleepMs)
{
    _worker = std::thread([=]
    {
        for ( ;; )
        {
            TaskExecute task;
            {

                std::unique_lock<std::mutex> lock( this->_queue_mutex );
                tick_t sleep_time = sleepMs;
                bool block = true;
                if (!this->_tasks.empty())
                {
                    sleep_time = _tasks.top().elapsed_ms - CurrentTimeMs();
                    if ( sleep_time < 3 )
                    {
                        task = std::move( this->_tasks.top().executer );
                        this->_tasks.pop();
                        block = false;
                    }
                }
                if (block)
                {
                    this->condition.wait_for( lock, std::chrono::milliseconds( sleep_time ),
                                              [this] { return this->_stop || !this->_tasks.empty(); } );
                    if ( this->_stop && this->_tasks.empty() )
                        return;
                }
            }
            if(task)task();
        }
    });
}

Timer::~Timer()
{
    {
        std::unique_lock<std::mutex> lock( _queue_mutex );
        _stop = true;
    }
    condition.notify_all();
    _worker.join();
}

void Timer::AddTask( int elapsed_ms, TaskExecute executer )
{
    std::unique_lock<std::mutex> lock( this->_queue_mutex );
    if ( _stop )
    {
        throw std::runtime_error("Add Task on stop Timer");
    }
    TimerTask task( elapsed_ms + CurrentTimeMs(), std::move( executer ) );
    _tasks.emplace( std::move( task ) );
}

void Timer::ClearTask()
{
    std::unique_lock<std::mutex> lock( this->_queue_mutex );
    while ( !_tasks.empty() )
    {
        _tasks.pop();
    }
}

