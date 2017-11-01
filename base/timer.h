#pragma once
#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

namespace audio_engine
{
	typedef  std::function<void() > TaskExecute;
	namespace detail
	{
		class STimer;
	}
	class TimerThread;
	class Timer
	{
	public:
		Timer( TimerThread* timer );
		~Timer();
		void AddTask( int elapsed_ms, TaskExecute&& executer );
	private:
		std::shared_ptr<detail::STimer> _timer_impl;
	};

	struct TimerTask;
	class TimerThread
	{
	public:
		TimerThread( int sleepMs = 100 );
		~TimerThread();
		void AddTask( int elapsed_ms, TaskExecute &&executer );
		void ClearTask();
	private:
		std::thread _worker;
		std::priority_queue<TimerTask> _tasks;
		std::mutex _queue_mutex;
		std::condition_variable condition;
		bool _stop;
	};
}