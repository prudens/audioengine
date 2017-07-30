#include "server_module.h"

#include "base/async_task.h"
#include "base/timer.h"
#include "real_audio_common.h"

#include "socket_manager.h"
#include "server_config.h"
#include "token_generater.h"


ServerModule* ServerModule::s_instance = nullptr;
ServerModule* ServerModule::GetInstance()
{
    return s_instance;
}

void ServerModule::CreateInstance()
{
    if (s_instance)
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


SocketManager* ServerModule::GetSocketManager()
{
    return _socket_mgr;
}

AsyncTask* ServerModule::GetAsyncTask()
{
    return _task;
}

Timer* ServerModule::GetTimer()
{
    return _timer;
}

ServerModule::ServerModule()
{
    _buffer_pool = new BufferPool;
    _task = new AsyncTask( 4 );
    _timer = new Timer;
    _socket_mgr = new SocketManager();
    _srv_cfg = new ServerConfig;
    _token_gen = new TokenGenerater;
}

ServerModule::~ServerModule()
{
    delete _socket_mgr;
    delete _timer;
    delete _task;
    delete _srv_cfg;
    delete _token_gen;
    delete _buffer_pool;
}

TokenGenerater* ServerModule::GetTokenGenerater()
{
    return _token_gen;
}

BufferPool* ServerModule::GetBufferPool()
{
    return _buffer_pool;
}


