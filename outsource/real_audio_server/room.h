#pragma once
#include<string>
#include<list>
#include"user_list.h"
#include"user_client.h"
namespace audio_engine{
	class Room
	{
	public:
		bool FindMember(std::string uid);
		void JoinMember( ConstMemberPtr member, UserClientPtr new_client );
		void LeaveMember( int64_t token, UserClientPtr client );
		void UpdateUserExtend(std::string extend, UserClient* client);
		void UpdateUserState(int state, UserClient*client);
		bool HandleConnection( UserConnPtr conn );
		std::vector<ConstMemberPtr> GetMemberList();
	private:
		MemberList _members;
		std::list<UserClientPtr> _clients;
		ProtoPacket _packet;
		std::list<UserClientPtr> _cache_clients;
	};
}