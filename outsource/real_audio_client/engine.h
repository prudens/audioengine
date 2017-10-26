#pragma once
#include "message_queue.h"
#include "client_manager.h"
#include <thread>
#include <string>

class Engine
{
public:
    Engine( std::string filename, std::string path );
    ~Engine();
    void Run();//À¿—≠ª∑
    void SendCmd();
private:
    MessageQueue* _queue;
    ClientManager* _client_mgr;
    std::thread _thread;
};

void run_engine( std::string filename,std::string path );