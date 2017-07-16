#include "task.h"
#include <chrono>
void Task::Start( int timeus )
{
    if (_run_flag)
    {
        return;
    }
    _run_flag = true;
    _timeus = timeus;
    _thread = std::thread( std::bind(&Task::Run,this) );
}

void Task::Stop()
{
    if (!_run_flag)
    {
        return;
    }
    _run_flag = false;
    if ( _thread.joinable() )
        _thread.join();
}

void Task::Run()
{
    while ( _run_flag )
    {
        bool sleep = true;
        _lock.lock();
        for ( auto p : _processers )
        {
            if ( p->Process() )
            {
                sleep = false;
            }
        }
        _lock.unlock();
        if (sleep)
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( _timeus ) );
        }
    }

}

void Task::AddProcesser( Processer * p)
{
    _lock.lock();
    _processers.push_back( p );
    _lock.unlock();
}

void Task::RemoveProcesser( Processer* p )
{
    _lock.lock();
    _processers.remove( p );
    _lock.unlock();
}
