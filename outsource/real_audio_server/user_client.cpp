#include "user_client.h"
#include "server_module.h"
#include "token_generater.h"
#include "room.h"
namespace audio_engine{
	UserClient::UserClient( Room* room, UserConnPtr user )
	{
		_room = room;
		_user_conn_ptr = user;
		_user_conn_ptr->SetPacketHandler( this );
	}

	UserClient::~UserClient()
	{
		_user_conn_ptr->SetPacketHandler( nullptr );
	}

	void UserClient::HandleError( std::error_code ec )
	{

	}

	void UserClient::HandlePacket( RAUserMessagePtr pb )
	{
		if(pb->has_request_login())
		{
			auto req_login = pb->request_login();
			ResponedLogin* res_login = pb->mutable_responed_login();
			res_login->set_userid( req_login.userid() );
			auto token = ServerModule::GetInstance()->GetTokenGenerater()->NewToken( req_login.userid() );
			res_login->set_token(token);
			res_login->set_error_code(0);
			
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
			auto member = CreateMember();
			member->SetToken( res_login->token() );
			member->SetUserID( req_login.userid() );
			member->SetDeviceType( req_login.devtype() );
			member->SetUserExtend( req_login.extend() );
			member->SetUserState( req_login.state() );

			pb->clear_request_login();
			SendToClient(pb);
			// 通知房间其他人
			auto self = shared_from_this();
			_room->JoinMember( member, self );
		}
	}

	void UserClient::SendToClient( RAUserMessagePtr pb )
	{
		_user_conn_ptr->SendPacket(pb);
	}

	void UserClient::SendToClient( BufferPtr buf )
	{
		_user_conn_ptr->Send( buf );
	}

	UserClientPtr CreateUserClient( Room* room, UserConnPtr user )
	{
		return std::make_shared<UserClient>( room, user );
	}
}