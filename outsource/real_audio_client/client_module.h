#pragma once
class SocketManager;
class ServerConfig;
class AsyncTask;
class Timer;
class ClientModule
{
public:
    static ClientModule* GetInstance();
    static void CreateInstance();
    static void DestroyInstance();
    
public:
    SocketManager* GetSocketManager();
    ServerConfig*  GetServerCnfig();
    AsyncTask*     GetAsyncTask();
    Timer*         GetTimer();
private:
    ClientModule();
    ~ClientModule();
    ClientModule( const ClientModule& ) = delete;
    ClientModule( const ClientModule&& ) = delete;
    static ClientModule* s_instance;
    SocketManager*       _socket_mgr = nullptr;
    ServerConfig*        _server_cfg = nullptr;
    AsyncTask*           _task = nullptr;
    Timer*               _timer = nullptr;
};