#pragma once
#include<memory>
#include "user_connection.h"
namespace audio_engine
{
	class Room;
	class UserClient: public std::enable_shared_from_this<UserClient>, public IPacketHandler
	{
	public:
		UserClient( Room* room, TaskThread&task, UserConnPtr user );
		~UserClient();
		void SendToClient( RAUserMessagePtr pb );
		void SendToClient( BufferPtr buf );
		int64_t Token()const;
	public:
		virtual void HandleError( std::error_code ec );
		virtual void HandlePacket( RAUserMessagePtr pb );
		void HandleLogin( RAUserMessagePtr pb );
		void HandleLogout( RAUserMessagePtr pb );
		void HandleUpdateExtend( RAUserMessagePtr pb );
		void HandleUpdateState( RAUserMessagePtr pb );
	private:
		AsyncTask _task;
		Room*       _room;
		UserConnPtr _user_conn_ptr;
		int64_t     _token  = 0;
		std::string _uid;
	};

	typedef std::shared_ptr<UserClient> UserClientPtr;
	UserClientPtr CreateUserClient( Room* room, TaskThread& thread, UserConnPtr user );
}