#pragma once
#include<memory>
#include "user_connection.h"
namespace audio_engine
{
	class Room;
	class UserClient:public IPacketHandler, std::enable_shared_from_this<UserClient>
	{
	public:
		UserClient( Room* room, UserConnPtr user );
		~UserClient();
		void SendToClient( RAUserMessagePtr pb );
		void SendToClient( BufferPtr buf );
	public:
		virtual void HandleError( std::error_code ec );
		virtual void HandlePacket( RAUserMessagePtr pb );

	private:
		Room*       _room;
		UserConnPtr _user_conn_ptr;
	};

	typedef std::shared_ptr<UserClient> UserClientPtr;
	UserClientPtr CreateUserClient( Room* room, UserConnPtr user );
}