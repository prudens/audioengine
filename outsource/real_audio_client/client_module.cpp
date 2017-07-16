#include "client_module.h"
#include "socket_manager.h"
#include "server_config.h"
#include "task.h"
ClientModule* ClientModule::s_instance = nullptr;

SocketManager* ClientModule::GetSocketManager()
{
    return _socket_mgr;
}

ServerConfig* ClientModule::GetServerCnfig()
{
    return _server_cfg;
}

Task* ClientModule::GetTask()
{
    _task->Start(10);
    return _task;
}

ClientModule::ClientModule()
{
    _socket_mgr = new SocketManager();
    _server_cfg = new ServerConfig();
    _task = new Task();
}

ClientModule::~ClientModule()
{
    delete _socket_mgr;
    delete _server_cfg;
}

ClientModule* ClientModule::GetInstance()
{
    return s_instance;
}

void ClientModule::CreateInstance()
{
    if (!s_instance)
    {
        s_instance = new ClientModule();
    }
}

void ClientModule::DestroyInstance()
{
    if (s_instance)
    {
        delete s_instance;
    }
}
