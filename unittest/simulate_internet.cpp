#include "simulate_internet.h"

class RoomServer;
std::unique_ptr<RoomServer> room_server;

void test_simulate_internet( int argc, char** argv )
{
    std::string server_ip("127.0.0.1");
    asio::io_context context;
    Server server(context,8888);
    auto f = std::async( [&] () { context.run(); } );
    tcp::endpoint ep( asio::ip::make_address( server_ip ), 8888 );

    Client*client[100];
    for ( int i = 0; i < 100; i++ )
    {
        client[i] = new Client( context, ep );
        client[i]->SetClientID( i+1 );
        client[i]->Login();
        std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
        client[i]->ReqVoip();
    }
    std::this_thread::sleep_for( std::chrono::seconds(60) );
    for ( int i = 0; i < 100;i++ )
    {
        delete client[i];
    }
    f.get();
    printf("end...");
}

RoomMember::RoomMember( tcp::socket socket, RoomServer& server ) :socket_( std::move( socket ) ),
host_( server )
{

}

void RoomMember::Start()
{
    Read();
}

void RoomMember::Read()
{
    auto self( shared_from_this() );
    socket_.async_read_some( asio::buffer( data_, max_length ),
                             [this, self] ( std::error_code ec, std::size_t length )
    {
        if ( !ec )
        {
            stVoipData* voipData = (stVoipData *)data_;
            uint32_t client_key = voipData->client_key;
            if ( key_ == 0 || key_ != client_key )
            {
                if ( !host_.QueuyValidKey( client_key ) )
                {
                    host_.RemoveRoomMember(self);
                    return;
                }
                else
                {
                    key_ = client_key;
                    host_.AddRoomMember( self );
                }
            }

            host_.RecvData( data_, length,self );
            Read();

        }
    } );
}

void RoomMember::Write( char* data, std::size_t length )
{
    asio::async_write( socket_, asio::buffer(data,length),
                       [this] (asio::error_code ec,std::size_t length)
    {
        if (ec)
        {
            printf( "[%u]–¥»Î ß∞‹£¨¥ÌŒÛ¬Î£∫%d\n",ec );
        }
    } );
}

void RoomMember::SetKey( uint32_t key )
{
    key_ = key;
}

uint32_t RoomMember::GetKey()
{
    return key_;
}

void RoomDataDispatch::AddMember( std::shared_ptr<RoomMember> member )
{
    members_.push_back( member );
}

void RoomDataDispatch::RemoveMember( std::shared_ptr<RoomMember> member )
{
    std::find( members_.begin(), members_.end(), member );
}

void RoomDataDispatch::Process( char*data, std::size_t length, std::shared_ptr<RoomMember> member )
{
    if ( length <= 4 )
    {
        return;
    }

    for ( auto&mem : members_ )
    {
        if ( mem->GetKey() == member->GetKey() )
        {
            continue;
        }
        mem->Write( data, length );
    }
}

RoomServer::RoomServer( asio::io_context& io_context, short port, uint32_t room_id ) : acceptor_( io_context, tcp::endpoint( tcp::v4(), port ) ),
socket_( io_context ),
room_id_( room_id )
{
    room_disptch_ = std::make_unique<RoomDataDispatch>();
    Accept();
}

void RoomServer::AddClientKey(uint32_t key)
{
    keys_.insert( key );
}

void RoomServer::AddRoomMember( std::shared_ptr<RoomMember> member )
{
    room_disptch_->AddMember( member );
}

void RoomServer::RemoveRoomMember( std::shared_ptr<RoomMember> member )
{
    room_disptch_->RemoveMember( member );
}

void RoomServer::RecvData( char* data, std::size_t length, std::shared_ptr<RoomMember> member )
{
    room_disptch_->Process( data, length, member );
}

bool RoomServer::QueuyValidKey( uint32_t key )
{
    return keys_.find( key ) != keys_.end();
}

void RoomServer::Accept()
{
    acceptor_.async_accept( socket_,
                            [this] ( std::error_code ec )
    {
        if ( !ec )
        {
            auto p = std::make_shared<RoomMember>( std::move( socket_ ), *this );
            p->Start();
        }

        Accept();
    } );
}

Session::Session( tcp::socket socket ) : socket_( std::move( socket ) )
{

}

void Session::Start()
{
    Read();
}

void Session::Read()
{
    auto self( shared_from_this() );
    socket_.async_read_some( asio::buffer( data_, max_length ),
                             [this, self] ( std::error_code ec, std::size_t length )
    {
        if ( !ec )
        {
            // process
            Process( length );
            Read();
        }
    } );
}

void Session::Write( char*data, std::size_t length )
{
    auto self( shared_from_this() );
    asio::async_write( socket_, asio::buffer( data, length ),
                       [this, self] ( std::error_code ec, std::size_t /*length*/ )
    {
        if ( ec )
        {
            printf( "[%u]server:–¥»Î ß∞‹£¨¥ÌŒÛ¬Î£∫%d", ec );
           // Read();
        }
    } );
}

void Session::Process( std::size_t length )
{
    stClientBase* pBuf = (stClientBase*)data_;
    if ( pBuf )
    {
        switch ( pBuf->msg_id )
        {
        case CLIENT_LOGIN:
        {
            stClientLogin*pLogin = (stClientLogin*)pBuf;
            _client_id = pLogin->client_id;
            stClientLoginResult* pLoginResult = (stClientLoginResult*)data_;
            pLoginResult->msg_id = CLIENT_LOGIN_RESULT;
            pLoginResult->client_id = _client_id;
            pLoginResult->ec = 0;
            pLoginResult->login_result = stClientLoginResult::eLoginSuc;
            Write( data_, sizeof( stClientLoginResult ) );
            break;
        }
        case CLIENT_REQ_SEND_VOIP_DATA:
        {
            stClientResVoipData res;
            res.ipv4 = asio::ip::make_address_v4( "127.0.0.1" ).to_uint();
            res.port = 8889;
            res.key = _client_id;
            if ( !room_server )
            {
                room_server = std::move( std::make_unique<RoomServer>( socket_.get_io_context(), 8889, 1 ) );
            }
            room_server->AddClientKey( _client_id );
            Write( (char*)&res, sizeof( res ) );
        }
        break;
        default:
            break;
        }
    }
}

Server::Server( asio::io_context& io_context, short port ) : acceptor_( io_context, tcp::endpoint( tcp::v4(), port ) ),
socket_( io_context )
{
    Accept();
}

void Server::Accept()
{
    acceptor_.async_accept( socket_,
                            [this] ( std::error_code ec )
    {
        printf( "coming a new connection...\n" );
        if ( !ec )
        {
            std::make_shared<Session>( std::move( socket_ ) )->Start();
        }

        Accept();
    } );
}

Client::Client( asio::io_context& io_context, tcp::endpoint ep ) :_socket( io_context )
{
    asio::error_code ec;
    _socket.connect( ep, ec );
    if ( ec )
    {
        printf( "¡¨Ω”∑˛ŒÒ∆˜ ß∞‹£∫¥ÌŒÛ¬Î:%d\n", ec );
    }
}

void Client::SetClientID( uint32_t client_id )
{
    client_id_ = client_id;
}

uint32_t Client::ClientID()
{
    return client_id_;
}

void Client::Write( stClientBase* client_base, std::size_t length )
{
    memcpy( data_, client_base, length );
    asio::async_write( _socket, asio::buffer( data_, length ),
                       [this, length] ( std::error_code ec, std::size_t len )
    {
        if ( !ec )
        {
            printf( "[%u]∑¢ÀÕ≥…π¶\n", client_id_ );
        }
        else
        {
            printf( "[%u]∑¢ÀÕ ß∞‹,¥ÌŒÛ¬Î£∫%d\n", client_id_, ec );
        }
    }
    );
}

void Client::Process( std::size_t length )
{
    stClientBase*client_base = (stClientBase*)rdata_;
    switch ( client_base->msg_id )
    {
    case CLIENT_LOGIN_RESULT:
    {
        stClientLoginResult*login_result = (stClientLoginResult*)client_base;
        assert( login_result->client_id == client_id_ );
        if ( login_result->login_result == stClientLoginResult::eLoginSuc )
        {
            printf( "[%u]µ«¬Ω≥…π¶\n", client_id_ );
        }
        else
        {
            printf( "[%u]µ«¬Ω ß∞‹,¥ÌŒÛ¬Î£∫%u\n", client_id_, login_result->ec );
        }
        //Logout();
    }
    break;
    case CLIENT_RES_SEND_VOIP_DATA:
    {
        printf( "[%u]ø…“‘∑¢”Ô“Ù ˝æ›¡À\n",client_id_ );
        stClientResVoipData* res = (stClientResVoipData*)rdata_;
        tcp::endpoint ep( asio::ip::address_v4(res->ipv4), res->port );
        channel_ = std::make_unique<Channel>(_socket.get_io_context(),ep);
        channel_->SetKey( res->key );
        channel_->Start();
    }
    break;
    default:
        Read( max_length );
        break;
    }
}


void Client::Login()
{
    stClientLogin login;
    login.client_id = client_id_;
    Write( &login, sizeof( login ) );
    Read( max_length );
}

void Client::Logout()
{
    asio::error_code ec;
    _socket.close( ec );
}

void Client::ReqVoip()
{
    stClientReqVoipData req;
    Write( &req, sizeof( req ) );
}

Channel::Channel( asio::io_context& io_context, tcp::endpoint ep ) :_socket( io_context )
, timer_( io_context )
{
    asio::error_code ec;
    _socket.connect( ep,ec );
    if ( ec )
    {
        printf( "¡¨Ω”∑˛ŒÒ∆˜≥ˆ¥Ì£¨¥ÌŒÛ¬Î£∫%d\n", ec );
    }
}

void Channel::SetKey( uint32_t key )
{
    key_ = key;
}

void Channel::Start()
{
    Read();
    Process();
}

void Channel::Process()
{
    timer_.expires_from_now( std::chrono::milliseconds( 2000 ) );
    timer_.async_wait( [this] ( asio::error_code ec )
    {

        stVoipData *data = (stVoipData*)data_;
        data->client_key = key_;
        data->len = 4;
        ( *(int*)data->data ) = count_++;
        if ( count_ > 1000 )
        {
            return;
        }
        Write(sizeof(stVoipData)+data->len);
        Process();
    } );
}

void Channel::Read()
{
    _socket.async_read_some(asio::buffer(rdata_, max_length),
                             [this] (asio::error_code ec,std::size_t length)
    {
        printf("[%u]recv data\n",key_);
        Read();
    } );
}

void Channel::Write(std::size_t length)
{
    asio::async_write( _socket, asio::buffer( data_, length ),
                       [this] ( asio::error_code ec, std::size_t length )
    {
        if (!ec)
        {
            printf("[%u]client:send data\n",key_);
        }
    } );
}
