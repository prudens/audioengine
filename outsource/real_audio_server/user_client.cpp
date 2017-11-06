#include "user_client.h"
#include "room.h"
UserClient::UserClient()
{

}

UserClient::~UserClient()
{

}

void UserClient::Initialize( Room* room, UserConnPtr user )
{
	_room = room;
	_user_conn_ptr = user;
}

void UserClient::ResponedVerifyAccount()
{

}

