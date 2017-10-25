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

enum LoginState{
    LS_NONE,          // 未连接服务器
	LS_CONNECTING,    // 正在连接服务器
	LS_CONNECTED,     // 已经连接服务器
	LS_VERIFY_ACCOUNT,// 连接成功，验证账号有效性
	LS_LOGOUT,        // 正在登出
	LS_LOGINED,       // 验证通过，登陆流程走完。
};


class UserEventHandler
{
public:
	virtual ~UserEventHandler(){}
	virtual void UpdateLoginState(LoginState state) = 0;
	virtual void UserEnterRoom(UserPtr user) = 0;
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
    int  Login(std::string userid);
	void Logout();
    int  GetLoginState();
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
	std::string _user_name;
	std::string _extend;
	int      _device_type;
	int64_t  _token;
	LoginState _cur_state = LS_NONE;
	LoginState _target_state = LS_NONE;
};