#include "io_context_pool.h"
#include <algorithm>


io_context_pool::io_context_pool( size_t pool_size )
{
    if (pool_size == 0)
    {
        pool_size = MIN_THREAD_NUM;
    }
    if ( pool_size > 10 )
    {
        pool_size = MAX_THREAD_NUM;
    }
    for ( size_t i = 0; i < pool_size; i++ )
    {
        io_context_ptr context =  std::make_shared<asio::io_context>();
        work_ptr work = std::make_shared<asio::io_context::work>( *context );
        io_contexts_.push_back( context );
        work_.push_back( work );
    }
    base_io_context_ = std::min( size_t(TASK_SOCKET), pool_size - 1 );
    next_io_context_ = base_io_context_;
}

io_context_pool::~io_context_pool()
{
    for ( auto & v : io_contexts_ )
    {
        v->stop();
    }
    for ( auto &v : threads_ )
    {
        v->join();
    }
}

void io_context_pool::run()
{
    for (auto& v: io_contexts_)
    {
        threads_.push_back(std::make_shared<std::thread>( [v] ()
        {
            v->run();
        } ) );
    }
}


io_context_pool::io_context_ptr io_context_pool::get_io_context( eTaskType type )
{
    if ( type == TASK_REALTIME)
    {
        io_context_ptr context = io_contexts_[next_io_context_];
        if (++next_io_context_ >= io_contexts_.size())
        {
            next_io_context_ = base_io_context_;
        }
        return context;
    }
    else
    {
        return io_contexts_[std::min(io_contexts_.size()-1,size_t(type-1))];
    }
}



