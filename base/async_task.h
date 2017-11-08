//ref link https://github.com/progschj/ThreadPool/blob/master/ThreadPool.h
#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
namespace audio_engine
{
	class TaskThread
	{
	public:
		TaskThread()
		{
			_worker = std::thread([this]
			{
				for(;; )
				{
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock( this->queue_mutex );
						this->condition.wait( lock,[this] { return stop || !tasks.empty(); } );
						if(stop && tasks.empty())
							return;
						task = std::move( tasks.front() );
						this->tasks.pop();
					}
					task();
				}
			});
		}

		TaskThread( TaskThread&& other )
		{
			this->_worker = std::move(other._worker );
			this->tasks = std::move(other.tasks);
			this->stop = other.stop;
		}
		template<class F>
		void AddTask( F&&task )
		{
			{
				std::unique_lock<std::mutex> lock( queue_mutex );

				// don't allow enqueueing after stopping the pool
				if(stop)
					throw std::runtime_error( "enqueue on stopped ThreadPool" );

				tasks.emplace( std::forward<F>( task ) );
			}
			condition.notify_one();
		}
		size_t TaskSize()const
		{
			std::unique_lock<std::mutex> lock( queue_mutex );
			return tasks.size();
		}
		~TaskThread()
		{
			{
				std::unique_lock<std::mutex> lock( queue_mutex );
				stop = true;
			}
			condition.notify_all();
			if(_worker.joinable())
				_worker.join();
		}
	private:
		// need to keep track of threads so we can join them
		std::thread _worker;
		// the task queue
		std::queue< std::function<void()> > tasks;

		// synchronization
		mutable std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop = false;
	};


	class ThreadPool
	{
	public:
		ThreadPool( size_t threads )
		{
			_workers.resize( threads );
		}
		~ThreadPool()
		{

		}
		TaskThread& GetTaskThread()
		{
			size_t size = _workers.size();
			size_t idx = 0;
			size_t num = _workers[idx].TaskSize();
			for(size_t i = 1; i < size; i++)
			{
				auto s = _workers[i].TaskSize();
				if(s < num)
				{
					num = s;
					idx = i;
				}
			}
			return _workers[idx];
		}
	private:
		std::vector<TaskThread> _workers;
	};
	class STask: public std::enable_shared_from_this<STask>
	{
	public:
		STask( ThreadPool* thread )
			:_thread(thread->GetTaskThread())
		{
		}
		STask( TaskThread&thread )
			:_thread(thread)
		{

		}
		template<class F, class... Args>
		auto AddTask( F&& f, Args&&... args )
			->std::future < typename std::result_of<F( Args... )>::type >
		{
			using return_type = typename std::result_of<F( Args... )>::type;

			auto task = std::make_shared< std::packaged_task<return_type()> >(
				std::bind( std::forward<F>( f ), std::forward<Args>( args )... )
				);

			std::future<return_type> res = task->get_future();
			auto self = shared_from_this();
			auto t = [=]()
			{
				self->_mutex.lock();
				if(!_stop)
				{
					(*task)();
				}
				self->_mutex.unlock();
			};
			_thread.AddTask(std::move(t));
			return res;
		}
		TaskThread& GetTaskThread()
		{
			return _thread;
		}
		void Stop()
		{
			_stop = true;
		}
	private:
		TaskThread& _thread;
		bool _stop = false;
		std::mutex _mutex;
	};

	class AsyncTask
	{
	public:
		AsyncTask( ThreadPool*pool )
		{
			_stask = std::make_shared<STask>(pool);
		}
		AsyncTask( TaskThread& thread )
		{
			_stask = std::make_shared<STask>(thread);
		}
		~AsyncTask()
		{
			if(_stask)
			{
				_stask->Stop();
				_stask.reset();
			}
		}
		template<class F, class... Args>
		auto AddTask( F&& f, Args&&... args )
			->std::future < typename std::result_of<F( Args... )>::type >
		{
			return _stask->AddTask(std::forward<F>(f),std::forward<Args>(args)...);
		}
		TaskThread& GetTaskThread()
		{
			return _stask->GetTaskThread();
		}
	public:
		AsyncTask( AsyncTask&&other )
		{
			_stask = other._stask;
			other._stask.reset();
		}
		AsyncTask( AsyncTask& ) = delete;
		AsyncTask& operator = ( AsyncTask& ) = delete;
	private:
		std::shared_ptr<STask> _stask;
	};
}
#endif