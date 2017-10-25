#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>
struct IUser
{
	virtual void SetUserID(std::string user_id ) = 0;
	virtual void SetUserName(std::string user_name) = 0;
	virtual void SetUserExtend(std::string extend) = 0;
	virtual void SetDeviceType(int dev_type) = 0;
	virtual void SetStatus(int status) = 0;

	virtual const std::string& GetUserID()const = 0;
	virtual const std::string& GetUserName()const = 0;
	virtual const std::string& GetUserExtend()const = 0;
	virtual int   GetDeviceType()const = 0;
	virtual int   GetStatus()const = 0;
	virtual void  CopyFrom(const IUser* user) = 0;
};

typedef std::shared_ptr<IUser> UserPtr;
typedef std::shared_ptr<const IUser> ConstUserPtr;
UserPtr CreateUser();
class UserList
{
public:
	bool Add(UserPtr ptr);
	bool Remove(std::string user_id);
	bool Update(std::string user_id, UserPtr ptr);
	bool Update(std::string user_id, std::string user_name);
	ConstUserPtr GetUser(std::string user_id)const;
	void Clear();
private:
	typedef std::unordered_map<std::string, UserPtr> UserMap;
	UserMap _users;
	std::mutex _mutex;
};