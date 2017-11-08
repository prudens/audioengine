#pragma once
#include <string>
#include <map>
#include "room.h"
#include "user_connection.h"
#include "client_conn_mgr.h"
namespace audio_engine{
	class MasterControl
	{
	public:
		MasterControl();
		~MasterControl();
		void Start();
		int JoinRoom( RAUserMessagePtr pb, UserConnPtr conn );
	private:
		typedef std::map<std::string, Room*> RoomList;
		RoomList _rooms;
		ClientConnMgr _conn_mgr;
	};
}