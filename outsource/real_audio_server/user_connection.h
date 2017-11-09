#pragma once
#include <memory>

#include "base/async_task.h"
#include "real_audio_common.h"
#include "protobuf_packet.h"
#include "user_service.pb.h"
#include "base/tcp_socket.h"
namespace audio_engine{
	typedef std::shared_ptr<RAUserMessage> RAUserMessagePtr;
	class UserConnection;
	class IPacketHandler
	{
	public:
		virtual void HandleError( std::error_code ec ) = 0;
		virtual void HandlePacket( RAUserMessagePtr pb ) = 0;
	};

	typedef std::shared_ptr<UserConnection> UserConnPtr;
	class UserConnection : public std::enable_shared_from_this<UserConnection>
	{
	public:
		UserConnection(TcpSocketPtr tcp, TaskThread&thread );
		~UserConnection();
		void SetVerifyAccountCB( std::function<int( RAUserMessagePtr pb, UserConnPtr conn )> cb );
		void SetPacketHandler( IPacketHandler * handler);
		void SendPacket( RAUserMessagePtr pb);
	public:
		void Start();
		void RecvPacket( std::error_code ec, RAUserMessagePtr pb );
		void VerifyFailed( RAUserMessagePtr pb,int ec );
		void Send( BufferPtr buf );
		void DettachTcp();
		void HandleError( std::error_code ec );
	private:
		void Read();
		void Write( BufferPtr buf );
		std::function<int( RAUserMessagePtr pb, UserConnPtr conn )> _VerifyAccount;
		AsyncTask _task;
		TcpSocketPtr _tcp_socket;
		ProtoPacket _proto_packet;
		bool _first_packet = true;
		bool _stop = false;
		IPacketHandler* _packet_handler = nullptr;
	};

}