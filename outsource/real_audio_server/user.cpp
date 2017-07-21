#include "user.h"
#include "server_module.h"
#include "socket_manager.h"
#include "user_service.pb.h"
#include "token_generater.h"
User::User()
    :_proto_packet(this)
{
    _sm = ServerModule::GetInstance()->GetSocketManager();
    _task = ServerModule::GetInstance()->GetAsyncTask();
}

User::~User()
{
    if (_sock_id>0)
    {
        DettachTcp();
    }
}

void User::AttachTcp( socket_t fd )
{
    _sock_id = fd;
    Read();
}

void User::DettachTcp()
{
    _sm->Cancel( _sock_id );
    _sock_id = 0;
}

void User::Read()
{
    if ( _sock_id == 0)
    {
        return;
    }
    auto self = shared_from_this();
    BufferPtr buf = _proto_packet.PullFromBufferPool();
    _sm->AsyncRead( _sock_id, buf->data, buf->capacity, [=] ( std::error_code ec, std::size_t length )
    {
        buf->length = length;
        if ( !ec )
        {
            self->_task->AndTask( [=]
            {
                self->_proto_packet.Parse( 0, buf );
            } );
            self->Read();
        }
        else
        {
            HandleError( ec );
        }
    } );
}

void User::Write(BufferPtr buf)
{
    auto self = shared_from_this();
    _sm->AsyncWrite( _sock_id, buf->data, buf->length, [=] ( std::error_code ec, std::size_t length )
    {
        self->_proto_packet.PushToBufferPool( buf );
        if ( ec )
        {
            self->HandleError( ec );
        }
    } );
}

void User::RecvPacket( int server_type, audio_engine::RAUserMessage* pb )
{
    if ( !pb)
    {
        return;
    }

    if ( pb->has_login_requst())
    {
        auto login_req = pb->login_requst();
        _userid = login_req.userid();
        _user_name = login_req.username();
        _extend = login_req.extends();
        _device_type = login_req.type();
        auto pb1 = _proto_packet.AllocProtoBuf();
        auto login_res = pb1->mutable_login_response();
        _token = ServerModule::GetInstance()->GetTokenGenerater()->NewToken( _userid );
        login_res->set_user_token(_token);
        login_res->set_userid( _userid );
        login_res->set_login_result( 1 );//fixme  改成枚举
        Send( pb1 );
    }
    else if ( pb->has_logout_requst())
    {
        auto logout_req = pb->logout_requst();
        auto token = logout_req.user_token();
        if ( token != _token)
        {
            printf( "无效请求，token不对" );
            return;
        }
        auto pb1 = _proto_packet.AllocProtoBuf();
        auto logout_res = pb1->mutable_logout_response();
        logout_res->set_user_token( _token );
        logout_res->set_logout_status( 1 );
        Send( pb1 );
    }
    else
    {
        printf("Unknown message");
    }
}

void User::SendPacket( int server_type, BufferPtr buf )
{
    Write( buf );
}

void User::HandleError( std::error_code ec )
{
    printf( ec.message().c_str() );
    printf( "\n" );
}

void User::Send( audio_engine::RAUserMessage* pb )
{
    BufferPtr buf = _proto_packet.Produce( 0, (const char*)&pb, sizeof( pb ) );
    auto self = shared_from_this();
    _task->AndTask( [=]
    {
        self->_proto_packet.Build( 0, buf );
    } );
}
