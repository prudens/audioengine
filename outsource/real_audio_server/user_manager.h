#pragma once
#include <system_error>
#include <list>
#include <memory>
#include "user.h"
#include "user_service.pb.h"
#include "base/tcp_socket.h"
namespace audio_engine{
	class UserManager :public std::enable_shared_from_this<UserManager>
	{
	public:
		UserManager();
		~UserManager();
	public:
		void Start();
		void Stop();
		void HandleLogin( std::shared_ptr<UserConnection> user );
		void HandleLogout( std::shared_ptr<UserConnection> user );
		void UpdateUserExtend( std::shared_ptr<UserConnection> user, std::shared_ptr< audio_engine::RAUserMessage> pb );
		void UpdateUserState( std::shared_ptr<UserConnection> user, std::shared_ptr< audio_engine::RAUserMessage> pb );
		const std::list<std::shared_ptr<UserConnection>>& GetUserList() { return _users; }
	private:
		bool HandleAccept( std::error_code ec, TcpSocketPtr tcp );
		TcpAcceptorPtr _acceptor;
		AsyncTask* _task = nullptr;
		ProtoPacket _packet;
		std::mutex _lock;
		std::list<std::shared_ptr<UserConnection>> _users;
		bool _stop = false;



	};
}