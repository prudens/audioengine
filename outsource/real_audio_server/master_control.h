#pragma once
#include <string>
#include <map>
#include "base/timer.h"
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
		void CheckRoom();
		
	private:
		typedef std::map<std::string, Room*> RoomList;
		std::mutex _mutex;
		RoomList _rooms;
		ClientConnMgr _conn_mgr;
		Timer   _timer;
	};
}