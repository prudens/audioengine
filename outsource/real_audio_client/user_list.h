#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <functional>
struct IMember
{
	virtual void  CopyFrom( const IMember* user ) = 0;
	virtual void SetUserID( std::string user_id ) = 0;
	virtual void SetToken( int64_t token ) = 0;
	virtual void SetUserExtend( std::string extend ) = 0;
	virtual void SetDeviceType( int dev_type ) = 0;
	virtual void SetUserState( int state ) = 0;
	virtual const std::string& GetUserID()const = 0;
	virtual const int64_t GetToken()const = 0;
	virtual const std::string& GetUserExtend()const = 0;
	virtual int   GetDeviceType()const = 0;
	virtual int   GetState()const = 0;
};

typedef std::shared_ptr<IMember> MemberPtr;
typedef std::shared_ptr<const IMember> ConstUserPtr;
MemberPtr CreateMember();
class MemberList
{
public:
	void UpdateList(std::vector<MemberPtr> users);
	bool Add(MemberPtr ptr);
	bool Remove(std::string user_id);
	bool Remove(int64_t token);
	bool Update(std::string user_id, MemberPtr ptr);
	bool Update(int64_t token, std::string user_extend);
	bool Update(int64_t token, int state);
	ConstUserPtr GetUser(std::string user_id)const;
	ConstUserPtr GetUser(int64_t token)const;
	void Traversal(std::function<void(ConstUserPtr)> cb);
	void Clear();
private:
	typedef std::unordered_map<std::string, MemberPtr> UserMap;
	typedef std::unordered_map<int64_t, std::string> TokenMap;
	UserMap _users;
	TokenMap _tokens;
	mutable std::mutex _mutex;
};