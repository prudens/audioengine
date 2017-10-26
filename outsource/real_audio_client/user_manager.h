#pragma once
#include <list>
#include "real_audio_common.h"
#include "protobuf_packet.h"
#include "user_service.h"
#include "base/timer.h"
#include "user_list.h"

//状态图
/*  LS_NONE----LS_CONNECTING-----LS_CONNECTED----LS_VERIFY_ACCOUNT
      \                                                           \
	   \													       \
		\												            LS_LOGINED
         \                                                         /
 		  \										                  /
           -----------------------LS_CONNECTED------LS_LOGOUT-----
*/														     
//凡是临时状态，都要加定时器，且值为奇数，方便过滤
enum LoginState{
    LS_NONE            =0,          // 未连接服务器
	LS_CONNECTING      = 1,         // 正在连接服务器
	LS_CONNECTED       = 2,         // 已经连接服务器
	LS_VERIFY_ACCOUNT  = 3,         // 连接成功，验证账号有效性
	LS_LOGINED         = 4,         // 验证通过，登陆流程走完。
	LS_LOGOUT          = 5,         // 正在执行登出操作
};


class UserEventHandler
{
public:
	virtual ~UserEventHandler(){}
	virtual void UpdateLoginState(LoginState state) = 0;
	virtual void UserEnterRoom(MemberPtr user) = 0;
	virtual void UserLeaveRoom(std::string user_id) = 0;
	virtual void UpdateUserState(std::string user_id, int state) = 0;
	virtual void UpdateUserExtend(std::string user_id, std::string extend) = 0;
};


class UserManager: ProtoPacketizer
{
public:
    UserManager( std::shared_ptr<UserService>  proto_packet = nullptr );
    ~UserManager();
public:
    void SetEventCallback(UserEventHandler* handler );
    int  Login(std::string userid,std::string roomkey);
	void Logout();
    int  GetCurState();
	int  GetTargetState();
	std::string GetUserID();
	std::string GetRoomKey();
private:
	void DoLogout();
	void ConnectServer();
	void DisConnectServer();
    void VerifyAccount();
	void OnTimer();
	void Transform(LoginState state);
public:
    virtual bool RecvPacket( std::shared_ptr<audio_engine::RAUserMessage> pb );
    virtual bool HandleError( int server_type, std::error_code ec );
    virtual bool HandleConnect( int server_type );
	void Update(LoginState state);
	UserEventHandler* _event_handle;
	UserServicePtr _user_service;
	STimerPtr   _timer;
	std::string  _user_id;
	std::string _roomkey;
	std::string _user_name;
	std::string _extend;
	int      _device_type;
	int64_t  _token;
	LoginState _cur_state = LS_NONE;
	LoginState _target_state = LS_NONE;
	uint64_t   _cur_state_time = 0;
};