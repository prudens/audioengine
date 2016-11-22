#pragma once
#include <memory>
#include "asio.hpp"
class io_context_pool
{
public:
    using io_context_ptr = std::shared_ptr< asio::io_context>;
    using work_ptr       = std::shared_ptr < asio::io_context::work > ;
    using thread_ptr     = std::shared_ptr < std::thread > ;
    enum eTaskType 
    {
        TASK_REALTIME = 0, // 实时
        TASK_UI,       // UI线程
        TASK_FILE,     // 文件
        TASK_SOCKET,   // 网络
    };
    enum
    {
        MIN_THREAD_NUM = 1,
        MAX_THREAD_NUM = 10,
    };
    io_context_pool( size_t pool_size );
    ~io_context_pool();
    void run();
    io_context_ptr get_io_context( eTaskType type );
private:
    /// The pool of io_services.   
    std::vector<io_context_ptr> io_contexts_;

    /// The work that keeps the io_services running.   
    std::vector<work_ptr> work_;
    std::vector<thread_ptr> threads_;
    size_t base_io_context_ = 0;
    size_t next_io_context_ = 3;
};