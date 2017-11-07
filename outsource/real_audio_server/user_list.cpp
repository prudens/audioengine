#include "user_list.h"
namespace audio_engine{
	struct MemberImpl : public IMember
	{
		virtual void SetUserID( std::string user_id )
		{
			_user_id = user_id;
		}
		virtual void SetToken( int64_t token )
		{
			_token = token;
		}
		virtual void SetUserExtend( std::string user_extend )
		{
			_user_extend = user_extend;
		}
		virtual void SetDeviceType( int dev_type )
		{
			_device_type = dev_type;
		}
		virtual void SetUserState( int state )
		{
			_state = state;
		}

		virtual const std::string& GetUserID()const
		{
			return _user_id;
		}
		virtual const int64_t GetToken()const
		{
			return _token;
		}
		virtual const std::string& GetUserExtend()const
		{
			return _user_extend;
		}
		virtual int GetDeviceType()const
		{
			return _device_type;
		}
		virtual int GetState()const
		{
			return _state;
		}

		virtual void CopyFrom( const IMember* user )
		{
			_user_id = user->GetUserID();
			_token = user->GetToken();
			_user_extend = user->GetUserExtend();
			_device_type = user->GetDeviceType();
			_state = user->GetState();
		}
	private:
		std::string _user_id;
		int64_t     _token;
		std::string _user_extend;
		int _device_type = 0;
		int _state = 0;

	};


	typedef std::unique_lock<std::shared_mutex> WriteLock;
	typedef std::shared_lock<std::shared_mutex> ReadLock;
	MemberPtr CreateMember()
	{
		return std::make_shared<MemberImpl>();
	}

	void MemberList::UpdateList( std::vector<ConstMemberPtr> users )
	{
		WriteLock lock( _mutex );
		_users.clear();
		_tokens.clear();
		for(auto & user : users)
		{
			_users[user->GetUserID()] = user;
			_tokens[user->GetToken()] = user->GetUserID();
		}
	}

	bool MemberList::Add( ConstMemberPtr ptr )
	{
		if(!ptr)
		{
			return false;
		}
		WriteLock lock( _mutex );
		_users[ptr->GetUserID()] = ptr;
		_tokens[ptr->GetToken()] = ptr->GetUserID();
		return true;
	}

	bool MemberList::Remove( std::string user_id )
	{
		WriteLock lock( _mutex );
		auto it = _users.find( user_id );
		if(it == _users.end())
		{
			return false;
		}
		_users.erase( it );
		return true;
	}

	bool MemberList::Remove( int64_t token )
	{
		WriteLock lock( _mutex );
		auto it = _tokens.find( token );
		if(it == _tokens.end())
		{
			return false;
		}
		std::string uid = it->second;
		auto itu = _users.find( uid );
		if(itu == _users.end())
		{
			return false;
		}
		_users.erase( itu );
		return true;
	}

	bool MemberList::Update( ConstMemberPtr ptr )
	{
		WriteLock lock( _mutex );
		auto it = _users.find( ptr->GetUserID() );
		if(it == _users.end())
		{
			return false;
		}

		it->second = ptr;
		return true;
	}

	bool MemberList::Update( int64_t token, std::string user_extend )
	{
		auto it = GetMember( token );
		if(!it)
		{
			return false;
		}
		MemberPtr new_user = CreateMember();
		new_user->SetDeviceType( it->GetDeviceType() );
		new_user->SetUserID( it->GetUserID() );
		new_user->SetUserExtend( user_extend );
		new_user->SetToken( it->GetToken() );
		Add( new_user );
		return true;
	}

	bool MemberList::Update( int64_t token, int state )
	{
		auto it = GetMember( token );
		if(!it)
		{
			return false;
		}
		MemberPtr new_user = CreateMember();
		new_user->SetDeviceType( it->GetDeviceType() );
		new_user->SetUserID( it->GetUserID() );
		new_user->SetToken( it->GetToken() );
		new_user->SetUserExtend( it->GetUserExtend() );
		new_user->SetUserState( state );
		Add( new_user );
		return false;
	}

	ConstMemberPtr MemberList::GetMember( std::string user_id )const
	{
		ReadLock lock( _mutex );
		auto it = _users.find( user_id );
		if(it == _users.end())
		{
			return nullptr;
		}
		return it->second;
	}

	ConstMemberPtr MemberList::GetMember( int64_t token )const
	{
		ReadLock lock( _mutex );
		auto it = _tokens.find( token );
		if(it == _tokens.end())
		{
			return nullptr;
		}
		auto v = _users.find( it->second );
		if(v == _users.end())
		{
			return nullptr;
		}
		return v->second;
	}

	std::string MemberList::GetUserID( int64_t token ) const
	{
		ReadLock lock( _mutex );
		auto it = _tokens.find( token );
		if(it == _tokens.end())
		{
			return "";
		}
		else
		{
			return it->second;
		}
	}

	void MemberList::Traversal( std::function<void( ConstMemberPtr )> cb )const
	{
		ReadLock lock( _mutex );
		for(auto item : _users)
		{
			cb( item.second );
		}
	}

	void MemberList::Clear()
	{
		WriteLock lock( _mutex );
		_users.clear();
		_tokens.clear();
	}

	size_t MemberList::Count()const
	{
		ReadLock lock( _mutex );
		return _users.size();
	}

}