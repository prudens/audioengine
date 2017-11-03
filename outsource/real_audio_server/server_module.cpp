#include "server_module.h"

#include "base/async_task.h"
#include "base/timer.h"
#include "real_audio_common.h"

#include "base/tcp_socket.h"
#include "server_config.h"
#include "token_generater.h"

namespace audio_engine{
	ServerModule* ServerModule::s_instance = nullptr;
	ServerModule* ServerModule::GetInstance()
	{
		return s_instance;
	}

	void ServerModule::CreateInstance()
	{
		if(s_instance)
		{
			return;
		}
		s_instance = new ServerModule;
	}

	void ServerModule::DestroyInstance()
	{
		delete s_instance;
		s_instance = nullptr;
	}

	ServerConfig* ServerModule::GetServerCnfig()
	{
		return _srv_cfg;
	}


	TcpFactory* ServerModule::GetSocketManager()
	{
		return _socket_mgr.get();
	}

	ThreadPool* ServerModule::GetThreadPool()
	{
		return _task;
	}

	TimerThread* ServerModule::GetTimerThread()
	{
		return _timer;
	}

	ServerModule::ServerModule()
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
		_task = new ThreadPool( 4 );
		_timer = new TimerThread;
		_socket_mgr = CreateTcpFactory( _io_context );
		_srv_cfg = new ServerConfig;
		_token_gen = new TokenGenerater;
	}

	ServerModule::~ServerModule()
	{
		delete _timer;
		delete _task;
		delete _srv_cfg;
		delete _token_gen;
		delete _buffer_pool;
		delete _work;
		_io_context.stop();
		_future.get();
	}

	TokenGenerater* ServerModule::GetTokenGenerater()
	{
		return _token_gen;
	}

	BufferPool* ServerModule::GetBufferPool()
	{
		return _buffer_pool;
	}


}