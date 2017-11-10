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
		 void UpdateLoginState( LoginState state );
		 void UserEnterRoom( ConstMemberPtr user );
		 void UserLeaveRoom( int64_t token );
		 void UpdateUserState( int64_t src_token, int64_t dst_token, int state, int ec );
		 void UpdateUserExtend( int64_t token, std::string extend, int ec );
		 void UpdateUserList( const std::vector<ConstMemberPtr>&users );
		 void KickoffUser( int64_t src_token, int64_t dst_token, int ec );
	public:
		void Login( std::string roomkey, std::string uid );
		void Logout();
		int  GetLoginState();
		void SetUserExtend( std::string extend );
		int  KickOff(std::string uid);
		IUserModuleSignal* GetUserModuleSignal();
		MemberList*        GetMemberList();
		boost::signals2::signal<void(std::string roomkey, std::string uid, int ec)> _RespondLogin;
		boost::signals2::signal<void( std::string roomkey, std::string uid, int ec )> _RespondLogout;
	private:
		MemberList _room_member_list;
		UserManager _user_mgr;
	};

}