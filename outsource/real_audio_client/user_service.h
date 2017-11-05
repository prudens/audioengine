#include <memory>
#include <unordered_map>
#include <boost/signals2.hpp>
#include "protobuf_packet.h"
#include "user_service.pb.h"
#include "base/tcp_socket.h"
#include "base/timer.h"
#include "base/async_task.h"
namespace audio_engine{
	typedef std::shared_ptr<RAUserMessage> RAUserMessagePtr;

	struct WaitRespPacket
	{
		RAUserMessagePtr pb;
		tick_t timeout;
		tick_t calltime;
		std::function<void( RAUserMessagePtr, std::error_code )> cb;
	};
	typedef std::shared_ptr<WaitRespPacket> WaitRespPacketPtr;
	class UserService : public std::enable_shared_from_this<UserService>
	{
	public:
		UserService();
		~UserService();
		void ConnectServer( std::string ip, int port );
		void DisconnectServer();
		bool IsConnectServer();
		void RecvPacket( std::error_code ec, std::shared_ptr<RAUserMessage> pb );
		void SendPacket( RAUserMessagePtr pb, tick_t timeout, std::function<void( RAUserMessagePtr, std::error_code )> cb );
		void SendPacket( RAUserMessagePtr pb );
		boost::signals2::signal<void( RAUserMessagePtr pb )> _RecvPacket;
		boost::signals2::signal<void( std::error_code ec )>  _HandleError;
		boost::signals2::signal<void( std::error_code ec )>  _HandleConnect;
	private:
		void     Read( BufferPtr buf );
		void     Write( BufferPtr buf );
		void     HandleConnect(std::error_code ec);
		void     TimerLoop();
		AsyncTask*    _task = nullptr;
		TcpSocketPtr _tcp_socket;
		ProtoPacket _proto_packet;
		BufferPool* _buffer_pool;

		int16_t        _sn;
		std::mutex _sns_mutex;
		Timer   _timer;
		std::unordered_map<int16_t, WaitRespPacketPtr> _req_packet_list;
	};

	typedef std::shared_ptr<UserService> UserServicePtr;
}