#include "server_config.h"
#include "socket_manager.h"
void ServerConfig::UpdateServerList()
{
    // load config from local file
}

int ServerConfig::GetServer( int type, std::string&ip, int16_t& port )
{
    for (auto & node:_server_list)
    {
        if ( node.type == type )
        {
            ip = node.ip;
            port = node.port;
            return 0;
        }   
    }
    return -1;
}

int ServerConfig::SetServer( int type, const std::string&ip, int16_t port )
{
    // 做域名解析工作。
    // 暂时先不做

    auto it = std::find_if( _server_list.begin(), _server_list.end(), [&] (const ServerNode& node)
    {
        return node.type == type;
    });

    if ( it != _server_list.end() )
    {
        it->ip = ip;
        it->port = port;
    }
    else
    {
        ServerNode node;
        node.ip = ip;
        node.type = type;
        node.port = port;
        _server_list.push_back( std::move(node) );
    }
    return 0;
}

void ServerConfig::RemoverServer( int type )
{
    auto it = std::find_if( _server_list.begin(), _server_list.end(), [&] ( const ServerNode& node )
    {
        return node.type == type;
    } );
    if (it != _server_list.end())
    {
        _server_list.erase( it );
    }
}

