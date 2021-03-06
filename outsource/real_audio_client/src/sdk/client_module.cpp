#include "client_module.h"

#include <stdio.h>

#include "base/tcp_socket.h"
#include "base/async_task.h"
#include "base/timer.h"
#include "server_config.h"
#include "real_audio_common.h"
namespace audio_engine{
	using namespace std::chrono;
	ClientModule* ClientModule::s_instance = nullptr;

	TcpFactory* ClientModule::GetTcpFactory()
	{
		return _socket_mgr.get();
	}

	ServerConfig* ClientModule::GetServerCnfig()
	{
		return _server_cfg;
	}

	ThreadPool* ClientModule::GetThreadPool()
	{
		return _task;
	}

	ClientModule::ClientModule()
	{
		_work = new asio::io_service::work( _io_context );
		_future = std::async( [&]() {
			std::error_code ec;
			_io_context.reset();
			_io_context.run( ec );
			if(ec)
			{
				printf( ec.message().c_str() );
			}
		} );

		_buffer_pool = new BufferPool;
		_socket_mgr = CreateTcpFactory( _io_context );
		_server_cfg = new ServerConfig();
		_task = new ThreadPool( 3 );
		_timer = new TimerThread(100ms);
	}

	ClientModule::~ClientModule()
	{
		delete _server_cfg;
		delete _task;
		delete _timer;
		delete _buffer_pool;
		delete _work;
		_io_context.stop();
		_future.get();
	}

	ClientModule* ClientModule::GetInstance()
	{
		return s_instance;
	}

	void ClientModule::CreateInstance()
	{
		if(!s_instance)
		{
			s_instance = new ClientModule();
		}
	}

	void ClientModule::DestroyInstance()
	{
		if(s_instance)
		{
			delete s_instance;
		}
	}

	TimerThread* ClientModule::GetTimerThread()
	{
		return _timer;
	}

// 	BufferPool* ClientModule::GetBufferPool()
// 	{
// 		return _buffer_pool;
// 	}
}
