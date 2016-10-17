#pragma once
#include "asio.hpp"
#include <chrono>
#include <map>
#include <memory>
#include <algorithm>
#include <set>
using asio::ip::tcp;
struct stClientBase
{
    stClientBase( uint32_t id ) :msg_id( id ) {}
    uint32_t msg_id;
};


#define CLIENT_LOGIN 0
struct stClientLogin :public stClientBase
{
    stClientLogin() :stClientBase( CLIENT_LOGIN ) {}
    uint32_t client_id;

};
#define CLIENT_LOGOUT 1
struct  stClientLogout :public stClientBase
{
    stClientLogout() :stClientBase( CLIENT_LOGOUT ) {}
    uint32_t client_id;
};

#define  CLIENT_LOGIN_RESULT 2
struct stClientLoginResult :public stClientBase
{
    enum{
        eLoginSuc,
        eLoginFail,
    };
    stClientLoginResult() :stClientBase( CLIENT_LOGIN_RESULT ) {}
    uint32_t client_id;
    uint32_t login_result = eLoginFail;
    uint32_t ec = 0;
};

#define CLIENT_REQ_SEND_VOIP_DATA 3
struct stClientReqVoipData :public stClientBase
{
    stClientReqVoipData() :stClientBase( CLIENT_REQ_SEND_VOIP_DATA ) {}
};

#define CLIENT_RES_SEND_VOIP_DATA 4
struct stClientResVoipData :public stClientBase
{
    stClientResVoipData() :stClientBase( CLIENT_RES_SEND_VOIP_DATA ) {}
    uint32_t ipv4;
    uint32_t port;
    uint32_t key;
};
struct stVoipData
{
    uint32_t client_key;
    std::size_t len;
    int count;
};

class RoomServer;
class RoomMember :public std::enable_shared_from_this<RoomMember>
{
public:
    RoomMember( tcp::socket socket, RoomServer& server );
    ~RoomMember();
    void Start();
    void Stop();
    void Read();
    void Write( char* data, std::size_t length );
    void SetKey( uint32_t key );
    uint32_t GetKey();
private:
    tcp::socket socket_;
    RoomServer& host_;
    uint32_t key_ = 0;
    enum { max_length = 1024 };
    char data_[max_length];
    char rdata_[max_length];
    bool stop_=false;
};


class RoomServer
{
public:
    RoomServer( asio::io_context& io_context, short port, uint32_t room_id );
    ~RoomServer();
    void Run()
    {
        Accept();
    }
    void AddClientKey(uint32_t key);
    void AddRoomMember( std::shared_ptr<RoomMember> member );
    void RemoveRoomMember(std::shared_ptr<RoomMember> member);
    void RecvData( char* data, std::size_t length, std::shared_ptr<RoomMember> member );
    bool QueuyValidKey( uint32_t key );

private:
    void Accept();

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    uint32_t room_id_;
    std::set<uint32_t> keys_;
    std::vector<std::shared_ptr<RoomMember> > members_;
};

class Session : public std::enable_shared_from_this < Session >
{
public:
    Session(asio::io_context&context, tcp::socket socket );
    ~Session();
    void Start();
private:
    void Read();

    void Write( char*data, std::size_t length );

    void Process( std::size_t length );
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    uint32_t _client_id = 0;
    asio::io_context& context_;
};
class Server
{
public:
    Server( asio::io_context& io_context, short port );
private:
    void Accept();
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    asio::io_context& context_;
};

class Channel;
class Client
{
public:
    Client( asio::io_context& io_context, tcp::endpoint ep );
    ~Client();
    void SetClientID( uint32_t client_id );
    uint32_t ClientID();
    void Write( stClientBase* client_base, std::size_t length );
    void Read( std::size_t length )
    {
        _socket.async_read_some( asio::buffer( rdata_, max_length ),
                                 [this] ( std::error_code ec, std::size_t length )
        {
            if ( !ec )
            {
                // process
                Process( length );
                Read( max_length );
            }
        } );
    }
    void Process( std::size_t length );
    void Login();
    void Logout();
    void ReqVoip();
    void Stop();
private:
    tcp::socket _socket;
    enum { max_length = 1024 };
    char data_[max_length];
    char rdata_[max_length];
    uint32_t client_id_;
    std::unique_ptr<Channel> channel_;
    asio::io_context& context_;
};

class Channel
{
public:
    Channel( asio::io_context& io_context, tcp::endpoint ep );
    ~Channel();
    void SetKey(uint32_t key);
    void Start();
    void Process();
    void Stop();
private:
    void Read();
    void Write(std::size_t length);
    tcp::socket _socket;
    enum { max_length = 1024 };
    char data_[max_length];
    char rdata_[max_length];
    asio::steady_timer timer_;
    uint32_t count_;
    uint32_t key_;
};