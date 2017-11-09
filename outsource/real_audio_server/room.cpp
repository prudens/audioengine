#include "room.h"
#include "server_module.h"
#include "error_code.h"
namespace audio_engine{
	Room::Room()
	:_task(ServerModule::GetInstance()->GetThreadPool())
	,_packet(nullptr)
	{
	}
	bool Room::FindMember(std::string uid)
	{
		if(_members.GetMember( uid ) != nullptr)
		{
			return true;
		}
		return false;
    }

	bool Room::FindMember( int64_t token )
	{
		if (_members.GetMember(token) != nullptr )
		{
			return true;
		}
		return false;
	}

	bool Room::JoinMember( ConstMemberPtr member, UserClientPtr new_client )
	{
		bool bfind = false;
		_cache_client_mutex.lock();
		for(auto it = _cache_clients.begin();it != _cache_clients.end(); it++)
		{
			if( *it == new_client)
			{
				_members.Add( member );
				_clients.push_back( new_client );
				_cache_clients.erase( it );
				bfind = true;
				break;
			}
		}
		_cache_client_mutex.unlock();

		if(!bfind)
		{
			//Log 
			return false;
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
		return true;
	}

	bool Room::HandleConnection( UserConnPtr conn )
	{
		UserClientPtr client = CreateUserClient( this, _task.GetTaskThread(), conn );
		_cache_client_mutex.lock();
		_cache_clients.push_back( client );
		_cache_client_mutex.unlock();
		return true;
	}

	bool Room::HandleKickOff( RAUserMessagePtr pb, UserClientPtr client )
	{
		pb->set_sn(0);
		auto kickoff = pb->mutable_kickoff_user();
		int64_t token = kickoff->dst_token();
		kickoff->set_error_code(ERR_OK);
		auto buf = _packet.Build( pb );
		for(auto it = _clients.begin(); it != _clients.end(); ++it)
		{
			if(*it != client)
			{
				( *it )->SendToClient( buf );
			}
		}

		_clients.remove_if( [token]( auto v ){ return v->Token() == token; } );
		_members.Remove( client->Token() );
		return true ;
	}

	std::vector<ConstMemberPtr> Room::GetMemberList()
	{
		std::vector<ConstMemberPtr> mem_list;
		mem_list.reserve( _members.Count() );
		_members.Traversal( [&mem_list](ConstMemberPtr member)
		{
			mem_list.push_back( member );
		} );
		return mem_list;
	}

	bool Room::IsEmpty()
	{
		std::unique_lock<std::mutex> lock(_cache_client_mutex);
		return _clients.empty() && _cache_clients.empty();
	}

	void Room::LeaveMember( UserClientPtr client )
	{
		if (_members.GetMember(client->Token()) == nullptr)
		{
			return;
		}
		auto pb = std::make_shared<RAUserMessage>();
		auto logout = pb->mutable_notify_logout();
		logout->set_token( client->Token() );
		auto buf = _packet.Build(pb);
		for(auto it = _clients.begin(); it != _clients.end(); ++it)
		{
			if(*it != client)
			{
				( *it )->SendToClient( buf );
			}
		}
		_clients.remove_if( [client]( const UserClientPtr& it ){return it == client; } );
		_members.Remove( client->Token() );

	}
	void Room::UpdateUserExtend( RAUserMessagePtr pb, UserClientPtr client )
	{
		pb->set_sn( 0 );
		bool ret = _members.Update(client->Token(),pb->update_user_extend().extend());
		if (!ret)
		{
			// log message
			return;
		}
		auto buf = _packet.Build( pb );
		for (auto it = _clients.begin(); it != _clients.end(); ++it)
		{
			if(*it != client)
			    ( *it )->SendToClient( buf );
		}
	}

	void Room::UpdateUserState( RAUserMessagePtr pb, UserClientPtr client )
	{
		pb->set_sn( 0 );
		if(!_members.Update( client->Token(), pb->update_user_state().state() ))
		{
			return;
		}
		auto buf = _packet.Build( pb );
		for(auto it = _clients.begin(); it != _clients.end(); ++it)
		{
			if(*it != client)
			    ( *it )->SendToClient( buf );
		}
	}
}