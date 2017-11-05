#pragma once
#include "user_list.h"
#include "user_manager.h"
namespace audio_engine{
	class MasterControl final
	{
	public:
		MasterControl();
		~MasterControl();
	public:
		void Initialize();
		void Terminate();

	protected://UserEventHandler
		virtual void UpdateLoginState( LoginState state );
		virtual void UserEnterRoom( ConstMemberPtr user );
		virtual void UserLeaveRoom( int64_t token );
		virtual void UpdateUserState( int64_t src_token, int64_t dst_token, int state, int ec );
		virtual void UpdateUserExtend( int64_t token, std::string extend, int ec );
		virtual void UpdateUserList( const std::vector<ConstMemberPtr>&users );
	public:
		void Login( std::string roomkey, std::string uid );
		void Logout();
		int  GetLoginState();
		void SetUserExtend( std::string extend );
		IUserModuleSignal* GetUserModuleSignal();
		MemberList*        GetMemberList();
		boost::signals2::signal<void(std::string roomkey, std::string uid, int ec)> _RespondLogin;
		boost::signals2::signal<void( std::string roomkey, std::string uid, int ec )> _RespondLogout;
	private:
		MemberList _room_member_list;
		UserManager _user_mgr;
	};

}