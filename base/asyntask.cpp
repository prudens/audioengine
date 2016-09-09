#include "asyntask.h"
#include "min_max_heap.hpp"
#include <list>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <atomic>

    uint64_t now()
    {
        return duration_cast<milliseconds>(
            system_clock::now() - time_point<system_clock>() ).count();
    }

    uint32_t AsynTask::s_task_id = 1;
    struct Task
    {

        uint32_t id;
        uint32_t elapsed;
        uint64_t expired;
        bool repeat;
        TaskExecute execute;
        Task() :id( 0 ),
            elapsed( 0 ),
            expired( (std::numeric_limits<uint64_t>::max)() ),
            repeat( false )
        {
        }

        bool operator < ( const Task& other )const
        {
            return this->elapsed < other.elapsed ||
                ( this->elapsed == other.elapsed
                && this->id < other.id );
        }
        bool operator >= ( const Task& other )const
        {
            return !(*this < other);
        }
    };

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
            for ( ;; )
            {
                for ( ;; )
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
                            task = _tasks.front();
                        }
                        if ( task.expired < t )
                        {
                            _tasks.pop_front();
                            if ( task.repeat )
                            {
                                task.expired = task.elapsed + t;
                                Insert( task );
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
                int32_t sleep_time = (std::numeric_limits<int32_t>::max)();

                std::unique_lock <std::mutex> lck( _lock );
                if ( !_tasks.empty() )
                {
                    sleep_time = static_cast<int32_t>( _tasks.front().expired - now() );
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
            Insert( task );
            _flag.store( true );
            _cond_variable.notify_all();
        }

        void Clear()
        {
            std::lock_guard<std::mutex> guard( _lock );
            _tasks.clear();
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
    protected:
        // Insert函数非线程安全
        void Insert(Task&task)
        {
            auto it = _tasks.begin();
            for ( ; it != _tasks.end(); ++it )
            {
                if ( task < *it )
                {
                    _tasks.insert( it, task );
                    break;
                }
            }
            if ( it == _tasks.end() )
            {
                _tasks.push_back( task );
            }
        }
    private:

        std::atomic_bool _stop;
        typedef std::list<Task> TaskList;
        TaskList _tasks;
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
