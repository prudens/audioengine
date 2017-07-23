#include "user.h"
#include "server_module.h"
#include "socket_manager.h"
#include "user_service.pb.h"
#include "token_generater.h"
#include "user_manager.h"
User::User( UserManager* host )
    :_proto_packet(this)
    , _host(host)
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
    BufferPtr buf = _proto_packet.PullFromBufferPool();
    buf->length = 0;
    Read(buf);
}

void User::DettachTcp()
{
    _stop = true;
    _sm->Cancel( _sock_id );
    _sock_id = 0;
}

UID User::userid()
{
    return _userid;
}

std::string User::username()
{
    return _user_name;
}

std::string User::extend()
{
    return _extend;
}

int User::device_type()
{
    return _device_type;
}

void User::Read(BufferPtr buf)
{
    if ( _stop )
    {
        return;
    }
    if ( _sock_id == 0)
    {
        return;
    }
    auto self = shared_from_this();
    _sm->AsyncRead( _sock_id, buf->data+buf->length, buf->capacity - buf->length,
                    [=] ( std::error_code ec, std::size_t length )
    {
        buf->length += length;
        if ( !ec )
        {
            if (buf->length < self->_proto_packet.header_size())
            {
                self->Read( buf );
            }
            else
            {
                auto content_length = self->_proto_packet.content_length( buf->data );
                auto packet_length = self->_proto_packet.header_size() + content_length;

                if ( buf->length >= packet_length )
                {
                    auto newbuf = _proto_packet.PullFromBufferPool();
                    newbuf->length = buf->length - packet_length;
                    memcpy( newbuf->data, buf->data + packet_length, newbuf->length );
                    self->_task->AndTask( [=]
                    {
                        self->_proto_packet.Parse( 0, buf );
                    } );
                    self->Read(newbuf);
                }
                else
                {
                    self->Read( buf );
                }
            }
        }
        else
        {
            HandleError( ec );
        }
    } );
}

void User::Write(int type, BufferPtr buf)
{
    if ( _stop )
    {
        return;
    }
    auto self = shared_from_this();
    _sm->AsyncWrite( _sock_id, buf->data, buf->length, [=] ( std::error_code ec, std::size_t length )
    {
        if(type == 0 )
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
        HandleLogin( login_req );
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
        Send(0, pb1 );
    }
    else
    {
        printf("Unknown message");
    }
}

void User::SendPacket( int server_type, BufferPtr buf )
{
    if ( server_type == 2)
    {
        if (_host)
        {
            auto self = shared_from_this();
            _host->HandleLogin( self, buf );
        }

    }
    else
    {
        Write( server_type, buf );
    }
}

void User::HandleError( std::error_code ec )
{
    if (ec)
    {
        printf( ec.message().c_str() );
        printf( "\n" );
        _stop;
        if (_host)
        {
            
        }
    }

}

void User::HandleLogin( const audio_engine::LoginRequest& login_req )
{
    _userid = login_req.userid();
    _user_name = login_req.username();
    _extend = login_req.extends();
    _device_type = login_req.type();
    auto pb1 = _proto_packet.AllocProtoBuf();
    auto login_res = pb1->mutable_login_response();
    _token = ServerModule::GetInstance()->GetTokenGenerater()->NewToken( _userid );
    login_res->set_user_token( _token );
    login_res->set_userid( _userid );
    login_res->set_login_result( 1 );//fixme  改成枚举
    Send(0, pb1 );
    auto pb2 = _proto_packet.AllocProtoBuf();
    auto login_ntf = pb2->mutable_login_notify();
    login_ntf->set_reason( 0 );
    login_ntf->set_userid(_userid);
    login_ntf->set_username( _user_name );
    login_ntf->set_extend( _extend );

    auto self = shared_from_this();
    Send( 2, pb2);
}

void User::Send(int type, audio_engine::RAUserMessage* pb )
{
    BufferPtr buf = _proto_packet.Produce( type, (const char*)&pb, sizeof( pb ) );
    auto self = shared_from_this();
    _task->AndTask( [=]
    {
        self->_proto_packet.Build( type, buf );
    } );
}
