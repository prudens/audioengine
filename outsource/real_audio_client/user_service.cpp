#include "user_service.h"

#include "client_module.h"
#include "base/async_task.h"
#include "base/common_defines.h"

UserService::UserService()
    :_proto_packet(std::bind(&UserService::RecvPacket,this,1,std::placeholders::_1,std::placeholders::_2))
{
    _buffer_pool = ClientModule::GetInstance()->GetBufferPool();
    _task = ClientModule::GetInstance()->GetAsyncTask();
}

UserService::~UserService()
{

}

void UserService::AddServer( int server_type, std::string ip, int port )
{
    auto self = shared_from_this();
	TcpFactory* fac = ClientModule::GetInstance()->GetTcpFactory();
	_tcp_socket = fac->CreateTcpConnection("", 0);
	ASSERT(_tcp_socket);
	_tcp_socket->AsyncConnect( ip, port, [=] ( std::error_code ec )
    {
        if ( !ec )
        {
            BufferPtr buf = _buffer_pool->PullFromBufferPool();
            self->Read( server_type, buf );
            _task->AddTask( [=]
            {
                self->HandleConnect( server_type );
            } );

        }
        else
        {
            self->HandleError( server_type, ec );
        }
    } );
}

void UserService::RemoveServer( int server_type )
{
	_tcp_socket->DisConnect();
	_tcp_socket.reset();
}

void UserService::RegisterHandler( ProtoPacketizer *p )
{
    _lock_handle.lock();
    _proto_handlers.push_back( p );
    _lock_handle.unlock();
}

void UserService::UnRegisterHandler( ProtoPacketizer* p )
{
    _lock_handle.lock();
    _proto_handlers.remove( p );
    _lock_handle.unlock();
}

void UserService::Read( int server_type, BufferPtr buf )
{
	if (!_tcp_socket)
	{
		return;
	}
    auto self = shared_from_this();
    _tcp_socket->AsyncRead( buf->WriteData(),buf->WriteAvailable(),
                            [=] ( std::error_code ec, size_t length )
    {
        if ( !ec )
        {
            buf->Write( length );
            self->_task->AddTask( [=]
            {
                self->_proto_packet.Parse( buf );
            } );
        }
        else
        {
            HandleError( server_type,ec );
        }
        auto buf = _buffer_pool->PullFromBufferPool();
        Read(server_type,buf);
    } );
}

void UserService::Write( int server_type, BufferPtr buf )
{
	if (!_tcp_socket)
	{
		return;
	}
    auto self = shared_from_this();
    _tcp_socket->AsyncWrite( buf->ReadData(),buf->ReadAvailable(),
                             [=] ( std::error_code ec, std::size_t length )
    {
        self->_buffer_pool->PushToBufferPool( buf );
        if (ec)
        {
            self->HandleError( server_type, ec );
        }
    } );
}


void UserService::HandleError( int server_type, std::error_code ec )
{
    _lock_handle.lock();
    for ( auto&p : _proto_handlers )
    {
        if ( p->HandleError( server_type, ec ) )
        {
            break;
        }
    }
    _lock_handle.unlock();
}

void UserService::HandleConnect( int server_type )
{
    _lock_handle.lock();
    for ( auto& p : _proto_handlers )
    {
        if ( p->HandleConnect( server_type ) )
        {
            break;
        }
    }
    _lock_handle.unlock();

}

void UserService::RecvPacket( int server_type, std::error_code ec, std::shared_ptr<audio_engine::RAUserMessage> pb )
{
    _lock_handle.lock();
    for ( auto& p:_proto_handlers)
    {
        if ( p->RecvPacket( pb ) )
        {
            break;
        }
    }
    _lock_handle.unlock();
}

void UserService::SendPacket( int server_type, std::shared_ptr<audio_engine::RAUserMessage> pb )
{
    auto buf = _proto_packet.Build( pb );
    if ( buf )
    {
        Write( server_type, buf );
    }
}
