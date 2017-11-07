#include "room.h"
namespace audio_engine{
	bool Room::FindMember(std::string uid)
	{
		if(_members.GetMember( uid ) != nullptr)
		{
			return true;
		}
		return false;
    }

	void Room::JoinMember( ConstMemberPtr member,UserClientPtr new_client )
	{
		_members.Add(member);
		for(auto it = _cache_clients.begin(); it != _cache_clients.end(); it++)
		{
			if( *it == new_client)
			{
				_clients.push_back(*it);
				_cache_clients.erase( it );
				break;
			}
		}
		//通知所有人新用户上线。
		auto pb = std::make_shared<RAUserMessage>();
		auto login_ntf = pb->mutable_notify_login();
		login_ntf->set_state( static_cast<audio_engine::USER_STATE>( member->GetState() ) );
		login_ntf->set_userid( member->GetUserID() );
		login_ntf->set_token( member->GetToken() );
		login_ntf->set_extend( member->GetUserExtend() );
		login_ntf->set_devtype( static_cast<audio_engine::DEVICE_TYPE>( member->GetDeviceType() ) );
		auto buf = _packet.Build( pb );
		for(auto it = _clients.begin(); it != _clients.end(); it++)
		{
			if(new_client == *it)
			{
				continue;
			}
			auto ptr = *it;
			ptr->SendToClient(buf);
		}
	}

	bool Room::HandleConnection( UserConnPtr conn )
	{
		UserClientPtr client = CreateUserClient( this, conn );
		_cache_clients.push_back( client );
		return true;
	}

	std::vector<ConstMemberPtr> Room::GetMemberList()
	{
		std::vector<ConstMemberPtr> mem_list( _members.Count() );
		_members.Traversal( [&mem_list](ConstMemberPtr member)
		{
			mem_list.emplace_back( member );
		} );
		return mem_list;
	}

	void Room::LeaveMember( int64_t token, UserClientPtr client )
	{
		_members.Remove( token );
		for(auto it = _clients.begin(); it != _clients.end(); ++it)
		{
			
		}
		_clients.remove_if( [client]( const UserClientPtr& it ){return it == client; } );

	}
	void Room::UpdateUserExtend( std::string extend, UserClient* client )
	{

	}
	void Room::UpdateUserState( int state, UserClient*client )
	{

	}
}