#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <functional>
struct IMember
{
	virtual void  CopyFrom(const IMember* user) = 0;
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
};

typedef std::shared_ptr<IMember> MemberPtr;
typedef std::shared_ptr<const IMember> ConstUserPtr;
MemberPtr CreateMember();
class MemberList
{
public:
	bool Add(MemberPtr ptr);
	bool Remove(std::string user_id);
	bool Update(std::string user_id, MemberPtr ptr);
	bool Update(std::string user_id, std::string user_extend);
	bool Update(std::string user_id, int state);
	ConstUserPtr GetUser(std::string user_id)const;
	void Traversal(std::function<void(ConstUserPtr)> cb);
	void Clear();
private:
	typedef std::unordered_map<std::string, MemberPtr> UserMap;
	UserMap _users;
	mutable std::mutex _mutex;
};