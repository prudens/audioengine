#pragma once
#include "master_control.h"
class Service
{
public:
	Service();
	~Service();
	void Run();
private:
	audio_engine::MasterControl* _master_ctrl;
};