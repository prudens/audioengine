#pragma once
#ifndef ASYN_TASK_H
#define ASYN_TASK_H
#include <limits>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>

namespace prudens
{
        using namespace  std::chrono;
        using namespace std::placeholders;
        class TaskQueue;
        uint64_t now();
        typedef  std::function<void( uint32_t ) > TaskExecute;
        struct Task
        {
#define MAX_MACRO_COMPILE_SUPPORT
            uint32_t id;
            uint32_t elapsed;
            uint64_t expired;
            bool repeat;
            TaskExecute execute;
            Task() :id( 0 ),
                elapsed( 0 ),
                expired( std::numeric_limits<uint64_t>::max MAX_MACRO_COMPILE_SUPPORT() ),
                repeat( false )
            {
            }

            bool operator < ( const Task& other )const
            {
                return this->elapsed < other.elapsed 
                    || this->elapsed == other.elapsed
                    && this->id < other.id;
            }
        };

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
}
#endif // !ASYN_TASK_H