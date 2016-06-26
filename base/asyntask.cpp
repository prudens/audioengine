#include "asyntask.h"
#include "min_max_heap.hpp"
namespace prudens
{
    uint32_t AsynTask::s_task_id = 1;
    class TaskQueue
    {
    public:
        TaskQueue()
        {
            _stop.store( false );
            _flag.store( false );
        }
        ~TaskQueue()
        {
            Stop();
        }
        void Run()
        {
            while ( true )
            {
                while ( true )
                {
                    if ( _stop )
                    {
                        return;
                    }
                    Task task;
                    uint64_t t = now();
                    {
                        std::lock_guard<std::mutex> guard( _lock );
                        if ( !_tasks.empty() )
                        {
                            task = _tasks.top();
                        }
                        if ( task.expired < t )
                        {
                            _tasks.pop();
                            if ( task.repeat )
                            {
                                task.expired = task.elapsed + t;
                                _tasks.push( task );
                            }
                        }
                        else
                        {
                            break;
                        }
                    }

                    if ( task.execute )
                    {
                        task.execute( task.id );
                    }
                }
                if ( _stop )
                {
                    return;
                }
                int32_t sleep_time = std::numeric_limits<int32_t>::max MAX_MACRO_COMPILE_SUPPORT();//列表为空的时候就睡眠30ms

                std::unique_lock <std::mutex> lck( _lock );
                if ( !_tasks.empty() )
                {
                    sleep_time = static_cast<int32_t>( _tasks.top().expired - now() );
                    if ( sleep_time < 0 )
                    {
                        continue;
                    }
                }
                if ( _flag.load() == false )
                {
                    _cond_variable.wait_for( lck, milliseconds( sleep_time ),
                                             [this] ()->bool { return this->_flag.load(); } );
                }
                if ( _stop )
                {
                    return;
                }
                _flag.store( false );
            }
        }

        void Push( Task& task )
        {
            std::unique_lock <std::mutex> lck( _lock );
            _tasks.push( task );
            _flag.store( true );
            _cond_variable.notify_all();

        }

        void Clear()
        {
            std::lock_guard<std::mutex> guard( _lock );
            while ( !_tasks.empty() )
            {
                _tasks.pop();
            }
        }

        void Remove( uint32_t task_id )
        {
            std::lock_guard<std::mutex> guard( _lock );
            _tasks.remove_if( [&] ( const Task& task )
            {
                return task.id == task_id;
            } );
        }

        void Stop()
        {
            std::unique_lock <std::mutex> lck( _lock );
            _flag.store( true );
            _stop.store( true );
            _cond_variable.notify_all();
        }
    private:
        std::atomic_bool _stop;
        max_min_heap<Task> _tasks;
        std::mutex _lock;
        std::condition_variable _cond_variable;
        std::atomic_bool _flag;
    };

    AsynTask::AsynTask()
    {
        _task_queue = std::make_shared<TaskQueue>();
        std::thread( [] ( std::shared_ptr<TaskQueue> queue )
        {
            queue->Run();
        }, _task_queue ).detach();
    }

    AsynTask::~AsynTask()
    {
        _task_queue->Stop();
    }

    uint32_t AsynTask::PostTask( int elapsed_ms, bool repeat, TaskExecute execute )
    {
        Task task;
        task.elapsed = elapsed_ms;
        task.expired = elapsed_ms + now();
        task.repeat = repeat;
        task.id = s_task_id++;
        task.execute = execute;
        _task_queue->Push( task );
        return task.id;
    }

    void AsynTask::RemoveTask( uint32_t task_id )
    {
        _task_queue->Remove( task_id );
    }

    void AsynTask::RemoveAllTask()
    {
        _task_queue->Clear();
    }

    uint64_t now()
    {
        return duration_cast<milliseconds>(
            system_clock::now() - time_point<system_clock>() ).count();
    }

}