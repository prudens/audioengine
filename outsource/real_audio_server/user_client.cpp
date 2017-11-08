#include "user_client.h"
#include "server_module.h"
#include "token_generater.h"
#include "room.h"
#include "error_code.h"
namespace audio_engine{
	UserClient::UserClient( Room* room, TaskThread&thread,UserConnPtr user )
		:_task(thread)
	{
		_room = room;
		_user_conn_ptr = user;
		if(_user_conn_ptr)_user_conn_ptr->SetPacketHandler( this );
	}

	UserClient::~UserClient()
	{
		if(_user_conn_ptr)_user_conn_ptr->SetPacketHandler( nullptr );
	}

	void UserClient::HandleError( std::error_code ec )
	{

	}

	void UserClient::HandlePacket( RAUserMessagePtr pb )
	{
		auto self = shared_from_this();
		if(pb->has_request_login())
		{
			_task.AddTask( [=]{
				self->HandleLogin( pb );
			} );
		}
		if ( pb->has_request_logout())
		{
			_task.AddTask( [=]{
				self->HandleLogout( pb );
			} );
		}

		if (pb->has_update_user_extend())
		{
			_task.AddTask( [=]{
				self->HandleUpdateExtend( pb );
			} );
		}
		 if ( pb->has_update_user_state())
		 {
			 _task.AddTask( [=]{
				 self->HandleUpdateState( pb );
			 } );
		 }
	}


	void UserClient::HandleLogin( RAUserMessagePtr pb )
	{
		bool sendall = true;
		auto req_login = pb->request_login();
		if (_token != 0 )
		{
			if ( req_login.userid() != _uid)
			{
				auto self = shared_from_this();
				_room->LeaveMember( self );
				sendall = true;
			}
			else
			{
				sendall = false;
			}
		}
		_uid = req_login.userid();
		if(sendall)
		{
			_token = ServerModule::GetInstance()->GetTokenGenerater()->NewToken( req_login.userid() );
		}

		ResponedLogin* res_login = pb->mutable_responed_login();
		res_login->set_userid( _uid );
		res_login->set_token( _token );
		res_login->set_error_code( 0 );

		NotifyUserList* user_list = pb->mutable_notify_user_list();
		auto members = _room->GetMemberList();

		for(const auto& user : members)
		{
			auto item = user_list->add_user();
			item->set_userid( user->GetUserID() );
			item->set_devtype( (DEVICE_TYPE)user->GetDeviceType() );
			item->set_extend( user->GetUserExtend() );
			item->set_token( user->GetToken() );
			item->set_state( user->GetState() );
		}
		user_list->set_pkg_flag( audio_engine::FLAG_LAST_PKG | audio_engine::FLAG_FIRST_PKG );
		pb->clear_request_login();
		SendToClient( pb );
		// 通知房间其他人
		if (sendall)
		{
			auto member = CreateMember();
			member->SetToken( res_login->token() );
			member->SetUserID( req_login.userid() );
			member->SetDeviceType( req_login.devtype() );
			member->SetUserExtend( req_login.extend() );
			member->SetUserState( req_login.state() );
			auto self = shared_from_this();
			_room->JoinMember( member, self );
		}

	}


	void UserClient::HandleLogout( RAUserMessagePtr pb )
	{
		auto req_logout = pb->request_logout();
		auto res_logout = pb->mutable_responed_logout();
		if(_token != req_logout.token() && _token != 0 )
		{
			res_logout->set_error_code( ERR_INVALID_AUTHO_KEY );
			pb->clear_request_logout();
			_user_conn_ptr->SendPacket( pb );
			return;
		}
		else
		{
			res_logout->set_error_code( ERR_OK );
			pb->clear_request_logout();
			_user_conn_ptr->SendPacket( pb );
		}
		auto self = shared_from_this();
		_room->LeaveMember( self);
		_token = 0;
	}


	void UserClient::HandleUpdateExtend( RAUserMessagePtr pb )
	{
		auto update_extend = pb->mutable_update_user_extend();
		if ( _token != update_extend->token() && _token != 0)
		{
			update_extend->set_error_code( ERR_INVALID_AUTHO_KEY );
			_user_conn_ptr->SendPacket( pb );
			return;
		}
		update_extend->set_error_code( ERR_OK );
		auto self = shared_from_this();
		_room->UpdateUserExtend( pb, self );
	}


	void UserClient::HandleUpdateState( RAUserMessagePtr pb )
	{
		auto update_state = pb->mutable_update_user_state();
		if(_token != update_state->src_token() && _token != 0)
		{
			update_state->set_error_code( ERR_INVALID_AUTHO_KEY );
			_user_conn_ptr->SendPacket( pb );
			return;
		}
		if (_room->FindMember(update_state->dst_token()))
		{
			update_state->set_error_code( ERR_USER_NOT_FOUND );
			_user_conn_ptr->SendPacket( pb );
			return;
		}
		auto self = shared_from_this();
		_room->UpdateUserState( pb, self);
	}

	void UserClient::SendToClient( RAUserMessagePtr pb )
	{
		_user_conn_ptr->SendPacket(pb);
	}

	void UserClient::SendToClient( BufferPtr buf )
	{
		_user_conn_ptr->Send( buf );
	}


	int64_t UserClient::Token() const
	{
		return _token;
	}

	UserClientPtr CreateUserClient( Room* room, TaskThread& thread, UserConnPtr user )
	{
		return std::make_shared<UserClient>( room, thread,user );
	}
}