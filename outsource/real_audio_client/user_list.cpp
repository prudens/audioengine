#include "user_list.h"

struct UserImpl : public IMember
{
	virtual void SetUserID(std::string user_id)
	{
		_user_id = user_id;
	}
	virtual void SetToken(int64_t token)
	{
		_token = token;
	}
	virtual void SetUserExtend(std::string user_extend)
	{
		_user_extend = user_extend;
	}
	virtual void SetDeviceType(int dev_type)
	{
		_device_type = dev_type;
	}
	virtual void SetUserState(int state)
	{
		_state = state;
	}

	virtual const std::string& GetUserID()const
	{
		return _user_id;
	}
	virtual const int64_t GetToken()const
	{
		return _token;
	}
	virtual const std::string& GetUserExtend()const
	{
		return _user_extend;
	}
	virtual int GetDeviceType()const
	{
		return _device_type;
	}
	virtual int GetState()const
	{
		return _state;
	}

	virtual void CopyFrom(const IMember* user)
	{
		_user_id = user->GetUserID();
		_token = user->GetToken();
		_user_extend = user->GetUserExtend();
		_device_type = user->GetDeviceType();
		_state = user->GetState();
	}
private:
	std::string _user_id;
	int64_t     _token;
	std::string _user_extend;
	int _device_type = 0;
	int _state = 0;

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
	_tokens[ptr->GetToken()] = ptr->GetUserID();
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

bool MemberList::Remove(int64_t token)
{
	lockGuard lock(_mutex);
	auto it = _tokens.find(token);
	if (it == _tokens.end())
	{
		return false;
	}
	std::string uid = it->second;
	_tokens.erase(it);
	Remove(uid);
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

bool MemberList::Update(int64_t token, std::string user_extend)
{
	auto it = GetUser(token);
	if (!it)
	{
		return false;
	}
	MemberPtr new_user = CreateMember();
	new_user->SetDeviceType( it->GetDeviceType());
	new_user->SetUserID(it->GetUserID());
	new_user->SetUserExtend(user_extend);
	new_user->SetToken(it->GetToken());
	Add(new_user);
	return true;
}

bool MemberList::Update(int64_t token, int state)
{
	auto it = GetUser(token);
	if (!it)
	{
		return false;
	}
	MemberPtr new_user = CreateMember();
	new_user->SetDeviceType(it->GetDeviceType());
	new_user->SetUserID(it->GetUserID());
	new_user->SetToken(it->GetToken());
	new_user->SetUserExtend(it->GetUserExtend());
	new_user->SetUserState(state);
	Add(new_user);
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

ConstUserPtr MemberList::GetUser(int64_t token)const
{
	lockGuard lock(_mutex);
	auto it = _tokens.find(token);
	if (it == _tokens.end())
	{
		return nullptr;
	}
	auto v = _users.find(it->second);
	if (v == _users.end())
	{
		return nullptr;
	}
	return v->second;
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