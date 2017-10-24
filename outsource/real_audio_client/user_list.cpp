#include "user_list.h"

bool UserList::Add(tUserPtr ptr)
{
	if (!ptr)
	{
		return false;
	}
	_users[ptr->user_id] = ptr;
	return true;
}

bool UserList::Remove(std::string user_id)
{
	auto it = _users.find(user_id);
	if ( it == _users.end())
	{
		return false;
	}
	_users.erase(it);
	return true;
}

bool UserList::Update(std::string user_id, tUserPtr ptr)
{
	auto it = _users.find(user_id);
	if (it == _users.end())
	{
		return false;
	}

	it->second = ptr;
	return true;
}

bool UserList::Update(std::string user_id, std::string user_name)
{
	auto it = _users.find(user_id);
	if (it == _users.end())
	{
		return false;
	}
	tUserPtr new_ptr = std::make_shared<tUser>();
	new_ptr->device_type = it->second->device_type;
	new_ptr->user_name = user_name;
	new_ptr->user_id = it->second->user_id;
	it->second = new_ptr;
	return true;
}

const tUserPtr UserList::GetUser(std::string user_id)const
{
	auto it = _users.find(user_id);
	if (it == _users.end())
	{
		return nullptr;
	}
	return it->second;
}

void UserList::Clear()
{
	_users.clear();
}
