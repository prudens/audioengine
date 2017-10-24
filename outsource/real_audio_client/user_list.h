#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>
struct tUser
{
	std::string user_id;
	std::string user_name;
	int         device_type;
};
typedef std::shared_ptr<tUser> tUserPtr;
class UserList
{
public:
	bool Add(tUserPtr ptr);
	bool Remove(std::string user_id);
	bool Update(std::string user_id, tUserPtr ptr);
	bool Update(std::string user_id, std::string user_name);
	const tUserPtr GetUser(std::string user_id)const;
	void Clear();
private:
	typedef std::unordered_map<std::string,tUserPtr> UserMap;
	UserMap _users;
	std::mutex _mutex;
};