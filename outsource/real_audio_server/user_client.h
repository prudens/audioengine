#pragma once
#include "user.h"
class Room;
class UserClient
{
public:
	UserClient();
	~UserClient();
	void Initialize(Room* room, UserConnPtr user);
	void ResponedVerifyAccount();
public:
	Room* _room;
	UserConnPtr _user_conn_ptr;
};