#include "user_list.h"

struct UserImpl : public IUser
{
	virtual void SetUserID(std::string user_id)
	{
		_user_id = user_id;
	}
	virtual void SetUserName(std::string user_name)
	{
		_user_name = user_name;
	}
	virtual void SetUserExtend(std::string user_extend)
	{
		_user_extend = user_extend;
	}
	virtual void SetDeviceType(int dev_type)
	{
		_device_type = dev_type;
	}
	virtual void SetStatus(int status)
	{
		_status = status;
	}

	virtual const std::string& GetUserID()const
	{
		return _user_id;
	}
	virtual const std::string& GetUserName()const
	{
		return _user_name;
	}
	virtual const std::string& GetUserExtend()const
	{
		return _user_extend;
	}
	virtual int GetDeviceType()const
	{
		return _device_type;
	}
	virtual int GetStatus()const
	{
		return _status;
	}

	virtual void CopyFrom(const IUser* user)
	{
		_user_id = user->GetUserID();
		_user_name = user->GetUserName();
		_user_extend = user->GetUserExtend();
		_device_type = user->GetDeviceType();
		_status = user->GetStatus();
	}
private:
	std::string _user_id;
	std::string _user_name;
	std::string _user_extend;
	int _device_type = 0;
	int _status = 0;

};


typedef std::lock_guard<std::mutex> lockGuard;
UserPtr CreateUser()
{
	return std::make_shared<UserImpl>();
}

bool UserList::Add(UserPtr ptr)
{
	if (!ptr)
	{
		return false;
	}
	lockGuard lock(_mutex);
	_users[ptr->GetUserID()] = ptr;
	return true;
}

bool UserList::Remove(std::string user_id)
{
	lockGuard lock(_mutex);
	auto it = _users.find(user_id);
	if ( it == _users.end())
	{
		return false;
	}
	_users.erase(it);
	return true;
}

bool UserList::Update(std::string user_id, UserPtr ptr)
{
	lockGuard lock(_mutex);
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
	lockGuard lock(_mutex);
	auto it = _users.find(user_id);
	if (it == _users.end())
	{
		return false;
	}
	UserPtr new_user = CreateUser();
	new_user->SetDeviceType( it->second->GetDeviceType());
	new_user->SetUserID(it->second->GetUserID());
	new_user->SetUserName(user_name);
	new_user->SetUserExtend(it->second->GetUserExtend());
	it->second = new_user;
	return true;
}

ConstUserPtr UserList::GetUser(std::string user_id)const
{
	lockGuard lock(_mutex);
	auto it = _users.find(user_id);
	if (it == _users.end())
	{
		return nullptr;
	}
	return it->second;
}

void UserList::Traversal(std::function<void(ConstUserPtr)> cb)
{
	lockGuard lock(_mutex);
	for (auto& item: _users)
	{
		cb(item.second);
	}
}

void UserList::Clear()
{
	lockGuard lock(_mutex);
	_users.clear();
}