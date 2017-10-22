#pragma once
#include <functional>
#include "asio.hpp"
class TcpFactory;
//class TcpFactoryPtr;
class ServerConfig;
class AsyncTask;
class Timer;
class BufferPool;
class ClientModule
{
public:
    static ClientModule* GetInstance();
    static void CreateInstance();
    static void DestroyInstance();
    
public:
	TcpFactory*      GetTcpFactory();
    ServerConfig*    GetServerCnfig();
    AsyncTask*       GetAsyncTask();
    Timer*           GetTimer();
    BufferPool*      GetBufferPool();
private:
    ClientModule();
    ~ClientModule();
    ClientModule( const ClientModule& ) = delete;
    ClientModule( const ClientModule&& ) = delete;
    static ClientModule* s_instance;
	std::shared_ptr<TcpFactory>          _socket_mgr = nullptr;
    ServerConfig*        _server_cfg = nullptr;
    AsyncTask*           _task = nullptr;
    Timer*               _timer = nullptr;
    BufferPool*          _buffer_pool = nullptr;
	asio::io_context     _io_context;
	asio::io_service::work* _work;
	std::future<void> _future;
};