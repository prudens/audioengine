#include "user_list.h"

struct UserImpl : public IMember
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

	virtual void CopyFrom(const IMember* user)
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
MemberPtr CreateMember()
{
	return std::make_shared<UserImpl>();
}

bool MemberList::Add(MemberPtr ptr)
{
	if (!ptr)
	{
		return false;
	}
	lockGuard lock(_mutex);
	_users[ptr->GetUserID()] = ptr;
	return true;
}

bool MemberList::Remove(std::string user_id)
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

bool MemberList::Update(std::string user_id, MemberPtr ptr)
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

bool MemberList::Update(std::string user_id, std::string user_extend)
{
	lockGuard lock(_mutex);
	auto it = _users.find(user_id);
	if (it == _users.end())
	{
		return false;
	}
	MemberPtr new_user = CreateMember();
	new_user->SetDeviceType( it->second->GetDeviceType());
	new_user->SetUserID(it->second->GetUserID());
	new_user->SetUserExtend(user_extend);
	new_user->SetUserName(it->second->GetUserName());
	it->second = new_user;
	return true;
}

bool MemberList::Update(std::string user_id, int state)
{
	lockGuard lock(_mutex);
	auto it = _users.find(user_id);
	if (it == _users.end())
	{
		return false;
	}
	MemberPtr new_user = CreateMember();
	new_user->SetDeviceType(it->second->GetDeviceType());
	new_user->SetUserID(it->second->GetUserID());
	new_user->SetUserName(it->second->GetUserName());
	new_user->SetUserExtend(it->second->GetUserExtend());
	new_user->SetStatus(state);
	it->second = new_user;
	return false;
}

ConstUserPtr MemberList::GetUser(std::string user_id)const
{
	lockGuard lock(_mutex);
	auto it = _users.find(user_id);
	if (it == _users.end())
	{
		return nullptr;
	}
	return it->second;
}

void MemberList::Traversal(std::function<void(ConstUserPtr)> cb)
{
	lockGuard lock(_mutex);
	for (auto& item: _users)
	{
		cb(item.second);
	}
}

void MemberList::Clear()
{
	lockGuard lock(_mutex);
	_users.clear();
}