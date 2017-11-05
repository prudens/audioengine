#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <functional>
namespace audio_engine{

	struct IMember
	{
		virtual void CopyFrom( const IMember* user ) = 0;
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
	typedef std::shared_ptr<const IMember> ConstMemberPtr;
	MemberPtr CreateMember();
	class MemberList
	{
	public:
		void UpdateList( std::vector<ConstMemberPtr> users );
		bool Add( ConstMemberPtr ptr );
		bool Remove( std::string user_id );
		bool Remove( int64_t token );
		bool Update( std::string user_id, MemberPtr ptr );
		bool Update( int64_t token, std::string user_extend );
		bool Update( int64_t token, int state );
		ConstMemberPtr GetMember( std::string user_id )const;
		ConstMemberPtr GetMember( int64_t token )const;
		std::string  GetUserID( int64_t token )const;
		void Traversal( std::function<void( ConstMemberPtr )> cb )const;
		void Clear();
		size_t Count()const;
	private:
		typedef std::unordered_map<std::string, ConstMemberPtr> MemberMap;
		typedef std::unordered_map<int64_t, std::string> TokenMap;
		MemberMap _users;
		TokenMap _tokens;
		mutable std::shared_mutex _mutex;
	};
}