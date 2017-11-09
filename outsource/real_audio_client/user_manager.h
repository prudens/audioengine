#pragma once
#include <list>
#include <mutex>
#include <atomic>

#include <boost/signals2.hpp>
#include "real_audio_common.h"
#include "protobuf_packet.h"
#include "user_service.h"
#include "base/timer.h"
#include "user_list.h"

//状态图
/*             LS_CONNECTING-----LS_CONNECTED-----LS_VERIFY_ACCOUNT
               /                                          \
	   		  /										       \
LS_NONE-----LS_INIT	                     			       LS_LOGINED
              \                                           /
 		       \									     /
               ---------LS_CONNECTED-------------LS_LOGOUT
*/														     
//凡是临时状态，都要加定时器，且值为奇数，方便过滤
namespace audio_engine{
	enum LoginState{
		LS_NONE = 0,
		LS_INIT = 2,         // 未连接服务器
		LS_CONNECTING = 3,         // 正在连接服务器
		LS_CONNECTED = 4,         // 已经连接服务器
		LS_VERIFY_ACCOUNT = 5,         // 连接成功，验证账号有效性
		LS_LOGINED = 6,         // 验证通过，登陆流程走完。
		LS_LOGOUT = 7,         // 正在执行登出操作

	};

#define MAX_TRY_LOGIN 5
	class IUserModuleSignal
	{
	public:
		boost::signals2::signal<void( LoginState state )> _UpdateLoginState;
		boost::signals2::signal<void( ConstMemberPtr user )> _UserEnterRoom;
		boost::signals2::signal<void( int64_t token )> _UserLeaveRoom;
		boost::signals2::signal<void( int64_t src_token, int64_t dst_token, int state, int ec )> _UpdateUserState;
		boost::signals2::signal<void( int64_t token, std::string extend, int ec )> _UpdateUserExtend;
		boost::signals2::signal<void( const std::vector<ConstMemberPtr>&users )> _UpdateUserList;
		boost::signals2::signal<void( int64_t src_token, int64_t dst_token, int ec )> _KickOffUserResult;
		virtual std::string GetUserID() = 0;
		virtual  std::string GetRoomKey() = 0;
		virtual int64_t GetToken() = 0;
	};


	class UserManager : public IUserModuleSignal
	{
	public:
		UserManager( std::shared_ptr<UserService>  proto_packet = nullptr );
		~UserManager();
	public:
		void        Login( std::string roomkey, std::string userid );
		void        Logout();
		void        SetUserExtend( std::string& extend );
		void        SetUserState( int64_t dst_token, int state );
		void        KickOffUser(int64_t token);
		int         GetCurState();
		int         GetTargetState();
		std::string GetUserID();
		std::string GetRoomKey();
		int64_t     GetToken();
		int         GetErrorCode()const{ return _error_code; }
	private:
		void DoLogout();
		void ConnectServer();
		void DisConnectServer();
		void VerifyAccount();
		void Update( LoginState state );
		void Transform( LoginState state );
		virtual void RecvPacket( std::shared_ptr<RAUserMessage> pb );
		virtual void HandleError( std::error_code ec );
		virtual void HandleConnect( std::error_code ec );
	private:
		UserServicePtr _user_service;
		AsyncTask*     _task = nullptr;
		Timer         _timer;
		std::string   _user_id;
		std::string   _roomkey;
		std::string   _extend;
		DEVICE_TYPE   _device_type = DEVICE_WINDOWS;
		int           _user_state = STATE_PLAYOUT;
		int64_t       _token = 0;
		LoginState    _cur_state = LS_NONE;
		std::atomic<LoginState> _target_state_internel = LS_NONE;
		LoginState    _target_state = LS_NONE;
		int           _cur_state_time = 0;
		uint32_t      _try_login_count = 0;
		int           _error_code = 0;
		std::vector<ConstMemberPtr> _cache_userlist;
		std::recursive_mutex        _mutex;
	};
}