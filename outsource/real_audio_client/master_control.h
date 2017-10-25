#pragma once
#include "user_list.h"
 class MasterControl final
{
public:
	MasterControl();
	~MasterControl();
public:
	void Initialize();
	void Terminate();
private:
	UserList _user_list;
	UserManager _user_mgr;
};

