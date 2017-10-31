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
	class Timer;

	//±ÿ–Î”√make_shared
	class STimer :public std::enable_shared_from_this<STimer>
	{
	public:
		STimer( Timer* timer );
		void AddTask( int elapsed_ms, TaskExecute executer );
		void Stop();
	private:

		Timer* _timer = nullptr;
		bool _stop = false;
		std::mutex _mutex;
	};
	typedef std::shared_ptr<STimer> STimerPtr;

	struct TimerTask;
	class Timer
	{
	public:
		Timer( int sleepMs = 100 );
		~Timer();
		void AddTask( int elapsed_ms, TaskExecute executer );
		void ClearTask();
	private:
		std::thread _worker;
		std::priority_queue<TimerTask> _tasks;
		std::mutex _queue_mutex;
		std::condition_variable condition;
		bool _stop;
	};

}