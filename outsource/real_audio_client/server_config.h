#pragma once
#include <string>
#include <vector>
#include <cstdint>
namespace audio_engine{
	class ServerConfig
	{
	public:
		ServerConfig();
		void UpdateServerList();
		bool GetServer( int type, std::string&ip, int16_t& port );
		int SetServer( int type, const std::string &ip, int16_t port );
		void RemoverServer( int type );
	private:
		struct ServerNode
		{
			std::string ip;
			int16_t port;
			int type;
		};
		std::vector<ServerNode> _server_list;
	};
}