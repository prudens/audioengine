#include "master_control.h"
#include <boost/signals2.hpp>
#include "base/log.h"
#include "SnailAudioEngineHelper.h"
namespace audio_engine{
	static audio_engine::Logger Log;
	MasterControl::MasterControl()
	{
		Log.setLevel( audio_engine::LEVEL_VERBOSE );
	}


	MasterControl::~MasterControl()
	{
	}

	void MasterControl::Initialize()
	{
	    namespace ph = std::placeholders;
		_user_mgr._UpdateLoginState.connect(0, std::bind(&MasterControl::UpdateLoginState,this,
			ph::_1));
		_user_mgr._UserEnterRoom.connect(0, std::bind( &MasterControl::UserEnterRoom, this,
			ph::_1 ) );
		_user_mgr._UserLeaveRoom.connect(0, std::bind( &MasterControl::UserLeaveRoom, this,
			ph::_1 ) );
		_user_mgr._UpdateUserExtend.connect(0, std::bind( &MasterControl::UpdateUserExtend, this,
			ph::_1, ph::_2, ph::_3 ) );
		_user_mgr._UpdateUserState.connect( 0, std::bind( &MasterControl::UpdateUserState, this,
			ph::_1, ph::_2, ph::_3, ph::_4 ) );
	    _user_mgr._UpdateUserList.connect(0, std::bind( &MasterControl::UpdateUserList, this,
			ph::_1 ) );
		_user_mgr._KickOffUserResult.connect( 0, std::bind( &MasterControl::KickoffUser, this,
			ph::_1, ph::_2, ph::_3 ) );
		
	}

	void MasterControl::Terminate()
	{
		_user_mgr._UpdateUserList.disconnect(0);
	}

	void MasterControl::UpdateLoginState( LoginState state )
	{
		// 只上报三种情况，登陆成功和失败，登出成功。但是内部要打印具体的操作
		if(state == LS_LOGINED && _user_mgr.GetTargetState() == LS_LOGINED)
		{
			_RespondLogin( _user_mgr.GetRoomKey().c_str(), _user_mgr.GetUserID().c_str(), _user_mgr.GetErrorCode() );
		}
		else if(state == LS_NONE && _user_mgr.GetTargetState() == LS_NONE)
		{
			_room_member_list.Clear();
			_RespondLogout( _user_mgr.GetRoomKey().c_str(), _user_mgr.GetUserID().c_str(), _user_mgr.GetErrorCode() );
		}
		else if(_user_mgr.GetTargetState() == LS_LOGINED && state == LS_NONE)
		{
			_RespondLogin( _user_mgr.GetRoomKey().c_str(), _user_mgr.GetUserID().c_str(), _user_mgr.GetErrorCode() );
		}
	}

	void MasterControl::UserEnterRoom( ConstMemberPtr user )
	{
		_room_member_list.Add( user );
	}

	void MasterControl::UserLeaveRoom( int64_t token )
	{
		_room_member_list.Remove( token );
	}

	void MasterControl::UpdateUserState( int64_t src_token, int64_t dst_token, int state, int ec )
	{
		if(_room_member_list.Update( dst_token, state ))
		{
			//这里处理其他模块逻辑
			if(src_token == _user_mgr.GetToken())
			{
				//说明是自己操作的，通知上层。
			}
			else
			{

			}
		}
	}

	void MasterControl::UpdateUserExtend( int64_t token, std::string extend, int ec )
	{
		Log.d( "MasterControl::UpdateUserExtend:%s\n", extend.c_str() );
		if(_room_member_list.Update( token, extend ))
		{
			//这里处理其他模块逻辑
			if(token == _user_mgr.GetToken())
			{
				//说明是自己设置的，通知上层。
				
			}
			else
			{

			}
		}
	}

	void MasterControl::UpdateUserList( const std::vector<ConstMemberPtr>& users )
	{
		_room_member_list.UpdateList( users );
	}

	void MasterControl::KickoffUser( int64_t src_token, int64_t dst_token, int ec )
	{
		if (dst_token == _user_mgr.GetToken())
		{
			_room_member_list.Clear();
		}
		else
		{
			_room_member_list.Remove( dst_token );
		}

	}

	void MasterControl::Login( std::string roomkey, std::string uid )
	{
		_user_mgr.Login( roomkey, uid );
	}

	void MasterControl::Logout()
	{
		_user_mgr.Logout();
	}

	int MasterControl::GetLoginState()
	{
		int state = _user_mgr.GetCurState();
		if(state == LS_LOGINED)
		{
			return 1;
		}
		else if(state == LS_NONE)
		{
			return 0;
		}
		else
		{
			return 2;
		}
	}

	void MasterControl::SetUserExtend( std::string extend )
	{
		_user_mgr.SetUserExtend( extend );
	}

	int MasterControl::KickOff( std::string uid )
	{
		auto it = _room_member_list.GetMember( uid );
		if(it == nullptr)
		{
			return ERR_USER_NOT_FOUND;
		}
		_user_mgr.KickOffUser(it->GetToken());
		return ERR_OK;
	}

	audio_engine::IUserModuleSignal* MasterControl::GetUserModuleSignal()
	{
		return &_user_mgr;
	}

	audio_engine::MemberList* MasterControl::GetMemberList()
	{
		return &_room_member_list;
	}

}