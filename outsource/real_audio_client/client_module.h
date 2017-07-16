#pragma once
class SocketManager;
class ServerConfig;
class Task;
class ClientModule
{
public:
    static ClientModule* ClientModule::GetInstance();
    static void ClientModule::CreateInstance();
    static void ClientModule::DestroyInstance();
    
public:
    SocketManager* GetSocketManager();
    ServerConfig*  GetServerCnfig();
    Task*          GetTask();
private:
    ClientModule();
    ~ClientModule();
    ClientModule( const ClientModule& ) = delete;
    ClientModule( const ClientModule&& ) = delete;
    static ClientModule* s_instance;
    SocketManager* _socket_mgr = nullptr;
    ServerConfig*  _server_cfg = nullptr;
    Task*          _task = nullptr;
};