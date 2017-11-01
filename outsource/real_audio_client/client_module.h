#pragma once
#include <functional>
#include <memory>
#include "asio.hpp"
class BufferPool;
namespace audio_engine{
	class TcpFactory;
	//class TcpFactoryPtr;
	class ServerConfig;
	class ThreadPoll;//#include"base/async_task.h"
	class TimerThread;
	class ClientModule
	{
	public:
		static ClientModule* GetInstance();
		static void CreateInstance();
		static void DestroyInstance();

	public:
		TcpFactory*      GetTcpFactory();
		ServerConfig*    GetServerCnfig();
		ThreadPoll*       GetThreadPool();
		TimerThread*     GetTimerThread();
		BufferPool*      GetBufferPool();
	private:
		ClientModule();
		~ClientModule();
		ClientModule( const ClientModule& ) = delete;
		ClientModule( const ClientModule&& ) = delete;
		static ClientModule* s_instance;
		std::shared_ptr<TcpFactory>          _socket_mgr = nullptr;
		ServerConfig*        _server_cfg = nullptr;
		ThreadPoll*           _task = nullptr;
		TimerThread*         _timer = nullptr;
		BufferPool*          _buffer_pool = nullptr;
		asio::io_context     _io_context;
		asio::io_service::work* _work;
		std::future<void> _future;
	};
}