#pragma once
#include <string>
#include "user_connection.h"
#include "base/tcp_socket.h"
#include "base/async_task.h"

namespace audio_engine{
	class MasterControl;
	class ClientConnMgr
	{
	public:
		ClientConnMgr(MasterControl* host);
		~ClientConnMgr();
		void StartListen();
		void StopListen();
		int VerifyAccount( RAUserMessagePtr pb, UserConnPtr conn );
		bool HandleAccept( std::error_code ec, TcpSocketPtr tcp );
	private:
		TcpAcceptorPtr _acceptor;
		AsyncTask _task;
		bool _stop = false;
		MasterControl *_master_ctrl;
	};
}