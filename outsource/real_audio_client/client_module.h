#pragma once
class TcpSocketManager;
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
    TcpSocketManager* GetSocketManager();
    ServerConfig*  GetServerCnfig();
    AsyncTask*     GetAsyncTask();
    Timer*         GetTimer();
    BufferPool*      GetBufferPool();
private:
    ClientModule();
    ~ClientModule();
    ClientModule( const ClientModule& ) = delete;
    ClientModule( const ClientModule&& ) = delete;
    static ClientModule* s_instance;
    TcpSocketManager*       _socket_mgr = nullptr;
    ServerConfig*        _server_cfg = nullptr;
    AsyncTask*           _task = nullptr;
    Timer*               _timer = nullptr;
    BufferPool*          _buffer_pool = nullptr;
};