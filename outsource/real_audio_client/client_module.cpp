#include "client_module.h"

#include "socket_manager.h"
#include "server_config.h"
#include "base/async_task.h"
#include "base/timer.h"
ClientModule* ClientModule::s_instance = nullptr;

TcpSocketManager* ClientModule::GetSocketManager()
{
    return _socket_mgr;
}

ServerConfig* ClientModule::GetServerCnfig()
{
    return _server_cfg;
}

AsyncTask* ClientModule::GetAsyncTask()
{
    return _task;
}

ClientModule::ClientModule()
{
    _buffer_pool = new BufferPool;
    _socket_mgr = new TcpSocketManager();
    _server_cfg = new ServerConfig();
    _task = new AsyncTask( 3 );
    _timer = new Timer;
}

ClientModule::~ClientModule()
{
    delete _socket_mgr;
    delete _server_cfg;
    delete _task;
    delete _timer;
    delete _buffer_pool;
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

Timer* ClientModule::GetTimer()
{
    return _timer;
}

BufferPool* ClientModule::GetBufferPool()
{
    return _buffer_pool;
}
