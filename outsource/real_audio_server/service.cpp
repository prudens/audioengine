#include "service.h"
#include "server_module.h"

Service::Service()
{
	audio_engine::ServerModule::CreateInstance();
	_master_ctrl = new audio_engine::MasterControl;
}

Service::~Service()
{
	delete _master_ctrl;
	audio_engine::ServerModule::DestroyInstance();
}

void Service::Run()
{
	_master_ctrl->Start();
}