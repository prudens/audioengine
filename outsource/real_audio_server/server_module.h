#pragma once
class SocketManager;
class AsyncTask;
class Timer;
class ServerConfig;
class TokenGenerater;
class ServerModule
{
public:
    static ServerModule* GetInstance();
    static void CreateInstance();
    static void DestroyInstance();
public:
    ServerConfig*    GetServerCnfig();
    SocketManager*   GetSocketManager();
    AsyncTask*       GetAsyncTask();
    Timer*           GetTimer();
    TokenGenerater*  GetTokenGenerater();
private:
    ServerModule();
    ~ServerModule();
    ServerModule( const ServerModule& ) = delete;
    ServerModule( ServerModule && ) = delete;
    ServerModule&operator=( const ServerModule& ) = delete;
    ServerModule&operator=( ServerModule&& ) = delete;
private:
    static ServerModule* s_instance;
    SocketManager*       _socket_mgr = nullptr;
    AsyncTask*           _task = nullptr;
    Timer*               _timer = nullptr;
    ServerConfig*        _srv_cfg = nullptr;
    TokenGenerater*      _token_gen = nullptr;
};