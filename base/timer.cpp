#include "timer.h"
#include "common_defines.h"
#include "time_cvt.hpp"
namespace audio_engine
{
	struct TimerTask
	{
	public:
		explicit TimerTask( tick_t elapsed_ms, TaskExecute executer )
		{
			this->elapsed_ms = elapsed_ms;
			this->executer = std::move( executer );
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


		tick_t elapsed_ms;
		TaskExecute executer;
	};

	TimerThread::TimerThread( tick_t sleepMs )
	{
		_worker = std::thread( [=]
		{
			for(;; )
			{
				TaskExecute task;
				{

					std::unique_lock<std::mutex> lock( this->_queue_mutex );
					tick_t sleep_time = sleepMs;
					bool block = true;
					if(!this->_tasks.empty())
					{
						using namespace std::chrono;
						sleep_time = _tasks.top().elapsed_ms - TimeStamp();
						if(sleep_time < 3ms)
						{
							task = std::move( this->_tasks.top().executer );
							this->_tasks.pop();
							block = false;
						}
					}
					if(block)
					{
						this->condition.wait_for( lock, sleep_time,[this] { return this->_stop; } );
						if(this->_stop && this->_tasks.empty())
							return;
					}
				}
				if(task)task();
			}
		} );
	}

	TimerThread::~TimerThread()
	{
		{
			std::unique_lock<std::mutex> lock( _queue_mutex );
			_stop = true;
		}
		condition.notify_all();
		_worker.join();
	}

	void TimerThread::AddTask( tick_t elapsed_ms, TaskExecute&& executer )
	{
		std::unique_lock<std::mutex> lock( this->_queue_mutex );
		if(_stop)
		{
			throw std::runtime_error( "Add Task on stop Timer" );
		}
		TimerTask task( elapsed_ms + TimeStamp(), std::move( executer ) );
		_tasks.emplace( std::move( task ) );
		condition.notify_all();
	}

	void TimerThread::ClearTask()
	{
		std::unique_lock<std::mutex> lock( this->_queue_mutex );
		while(!_tasks.empty())
		{
			_tasks.pop();
		}
	}
	namespace detail
	{
		//±ÿ–Î”√make_shared
		class STimer :public std::enable_shared_from_this<STimer>
		{
		public:
			STimer( TimerThread* timer );
			void AddTask( tick_t elapsed_ms, TaskExecute&& executer );
			void Stop();
		private:
			TimerThread* _timer = nullptr;
			bool _stop = false;
			std::mutex _mutex;
		};
		typedef std::shared_ptr<STimer> STimerPtr;

		void STimer::AddTask( tick_t elapsed_ms, TaskExecute&& executer )
		{
			if(_stop || !executer)
			{
				return;
			}

			auto self = shared_from_this();
			_timer->AddTask( elapsed_ms, [=]
			{
				_mutex.lock();
				if(!self->_stop)
				{
					executer();
				}
				_mutex.unlock();
			} );
		}

		void STimer::Stop()
		{
			_mutex.lock();
			_stop = true;
			_mutex.unlock();
		}

		STimer::STimer( TimerThread* timer )
		{
			ASSERT( timer );
			_timer = timer;
		}
	}


	Timer::Timer( TimerThread * timer )
	{
		_timer_impl = std::make_shared<detail::STimer>( timer );
	}

	Timer::~Timer()
	{
		_timer_impl->Stop();
		_timer_impl.reset();
	}

	void Timer::AddTask( tick_t elapsed_ms, TaskExecute&& executer )
	{
		_timer_impl->AddTask( elapsed_ms, std::move( executer ) );
	}

}