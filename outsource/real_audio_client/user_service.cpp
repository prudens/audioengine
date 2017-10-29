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

void UserService::ConnectServer( int server_type, std::string ip, int port )
{
	_sns_mutex.lock();
	_sns.clear();
	_sns_mutex.unlock();
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

void UserService::DisconnectServer( int server_type )
{
	if (_tcp_socket)
	{
		_tcp_socket->DisConnect();
		_tcp_socket.reset();
		_sns_mutex.lock();
		_sns.clear();
		_sns_mutex.unlock();
	}

}

bool UserService::IsConnectServer()
{
	return _tcp_socket != nullptr;
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

bool UserService::RemoveSn(int16_t sn)
{
	auto it = std::find(_sns.begin(), _sns.end(), sn);
	if (it == _sns.end())
	{
		return false;
	}
	_sns.erase(it);
	return true;
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
			auto buf = _buffer_pool->PullFromBufferPool();
			Read(server_type, buf);
        }
        else
        {
            HandleError( server_type,ec );
        }

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
	_sns_mutex.lock();
	int16_t sn = pb->sn();
	if (sn > 0)
	{
		auto it = std::find(_sns.begin(), _sns.end(), sn);
		if ( it == _sns.end())
		{
			printf("超时被忽略的包。\n");
			return;
		}
		else
		{
			_sns.erase(it);
		}
	}
	_sns_mutex.unlock();
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

int16_t UserService::SendPacket( int server_type, std::shared_ptr<audio_engine::RAUserMessage> pb )
{
	int sn = 0;
	_sns_mutex.lock();
	if (++_sn < 0)
	{
		_sn = 1;
	}
	_sns.push_back(_sn);
	_sn = _sn;
	_sns_mutex.unlock();
	pb->set_sn(_sn);
    auto buf = _proto_packet.Build( pb );
    if ( buf )
    {
        Write( server_type, buf );
    }

	return _sn;
}
