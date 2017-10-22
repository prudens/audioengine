#pragma once
#include <functional>
#include "asio.hpp"

class TcpFactory;
class AsyncTask;
class Timer;
class ServerConfig;
class TokenGenerater;
class BufferPool;
class ServerModule
{
public:
    static ServerModule* GetInstance();
    static void CreateInstance();
    static void DestroyInstance();
public:
    ServerConfig*    GetServerCnfig();
	TcpFactory*      GetSocketManager();
    AsyncTask*       GetAsyncTask();
    Timer*           GetTimer();
    TokenGenerater*  GetTokenGenerater();
    BufferPool*      GetBufferPool();
private:
    ServerModule();
    ~ServerModule();
    ServerModule( const ServerModule& ) = delete;
    ServerModule( ServerModule && ) = delete;
    ServerModule&operator=( const ServerModule& ) = delete;
    ServerModule&operator=( ServerModule&& ) = delete;
private:
    static ServerModule* s_instance;
    std::shared_ptr<TcpFactory>  _socket_mgr;
    AsyncTask*           _task = nullptr;
    Timer*               _timer = nullptr;
    ServerConfig*        _srv_cfg = nullptr;
    TokenGenerater*      _token_gen = nullptr;
    BufferPool*          _buffer_pool = nullptr;
	asio::io_context     _io_context;
	asio::io_service::work* _work;
	std::future<void> _future;
};