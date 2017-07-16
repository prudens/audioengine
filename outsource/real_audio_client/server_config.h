#pragma once
#include <string>
#include <vector>
#include <cstdint>

class ServerConfig
{
public:
    void UpdateServerList();
    int GetServer(int type,std::string&ip, int16_t& port);
    int SetServer(int type,const std::string &ip,int16_t port);
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