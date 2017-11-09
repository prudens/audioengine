#pragma once
#include<string>
#include<list>
#include <mutex>
#include "base/async_task.h"
#include"user_list.h"
#include"user_client.h"
namespace audio_engine{
	class Room
	{
	public:
		Room();
		bool FindMember(std::string uid);
		bool FindMember( int64_t token );
		bool JoinMember( ConstMemberPtr member, UserClientPtr new_client );
		void LeaveMember( UserClientPtr client );
		void UpdateUserExtend( RAUserMessagePtr pb, UserClientPtr client);
		void UpdateUserState( RAUserMessagePtr pb,UserClientPtr client);
		bool HandleConnection( UserConnPtr conn );
		bool HandleKickOff( RAUserMessagePtr pb, UserClientPtr client );
		std::vector<ConstMemberPtr> GetMemberList();
		bool IsEmpty();
	private:
		AsyncTask _task;
		MemberList _members;
		std::list<UserClientPtr> _clients;
		ProtoPacket _packet;
		std::list<UserClientPtr> _cache_clients;
		std::mutex               _cache_client_mutex;
	};
}