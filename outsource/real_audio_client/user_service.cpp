#include "user_service.h"

#include "client_module.h"
#include "socket_manager.h"
#include "base/async_task.h"

UserService::UserService()
    :_proto_packet(this)
{
    _task = ClientModule::GetInstance()->GetAsyncTask();
    _socket_mgr = ClientModule::GetInstance()->GetSocketManager();
}

UserService::~UserService()
{

}

void UserService::AddServer( int server_type, std::string ip, int port )
{
    if ( 0 != GetSocket( server_type ) )
    {
        return;
    }
    auto self = shared_from_this();
    _socket_mgr->AsyncConnect( ip, port, [=] ( std::error_code ec, socket_t socket_id )
    {
        if ( !ec )
        {
            BufferPtr buf = _proto_packet.PullFromBufferPool();
            buf->length = 0;
            self->SetSocket( server_type, socket_id );
            self->Read( server_type, buf );
            self->HandleConnect( server_type );
        }
        else
        {
            self->HandleError( server_type, ec );
        }
    } );
}

void UserService::RemoveServer( int server_type )
{
    _lock_sockets.lock();
    auto it = _sockets.find( server_type );
    if ( it != _sockets.end() )
    {
        _socket_mgr->DisConnect( it->second );
        _sockets.erase( it );
    }
    _lock_sockets.unlock();
}

void UserService::Produce( int server_type, void* data, size_t length )
{
    BufferPtr buf = _proto_packet.Produce( server_type, (const char*)data, length );
    auto self = shared_from_this();
    _task->AndTask( [=]
    {
        self->_proto_packet.Build( server_type, buf );
    } );
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

audio_engine::RAUserMessage* UserService::AllocProtoBuf()
{
    return _proto_packet.AllocProtoBuf();
}

void UserService::FreeProtobuf( audio_engine::RAUserMessage* pb )
{
    _proto_packet.FreeProtobuf( pb );
}

void UserService::Read( int server_type, BufferPtr buf )
{
    socket_t fd = GetSocket( server_type );
    if (fd == 0)
    {
        return;
    }
    auto self = shared_from_this();
    _socket_mgr->AsyncRead( fd, buf->data+buf->length, buf->capacity - buf->length,
                            [=] ( std::error_code ec, size_t length )
    {
        buf->length += length;
        if ( !ec )
        {
            if ( buf->length < self->_proto_packet.header_size() )
            {
                self->Read( server_type,buf );
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
                        self->_proto_packet.Parse( server_type, buf );
                    } );
                    self->Read( server_type,newbuf );
                }
                else
                {
                    self->Read(server_type, buf );
                }
            }
        }
        else
        {
            HandleError( server_type,ec );
        }
    } );
}

void UserService::Write( int server_type, BufferPtr buf )
{
    socket_t fd = GetSocket( server_type );
    if (0 == fd)
    {
        return;
    }
    auto self = shared_from_this();
    _socket_mgr->AsyncWrite( fd, buf->data, buf->length, [=] ( std::error_code ec, std::size_t length )
    {
        self->_proto_packet.PushToBufferPool( buf );
        if (ec)
        {
            self->HandleError( server_type, ec );
        }
    } );
}


socket_t UserService::GetSocket( int server_type )
{
    std::unique_lock<std::mutex> lock( _lock_sockets );
    auto it = _sockets.find( server_type );
    if ( it != _sockets.end() )
    {
        return it->second;
    }
    return 0;
}

void UserService::SetSocket( int server_type, socket_t fd )
{
    _lock_sockets.lock();
    _sockets[server_type] = fd;
    _lock_sockets.unlock();
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

void UserService::RecvPacket( int server_type, audio_engine::RAUserMessage* msg )
{
    _lock_handle.lock();
    for ( auto& p:_proto_handlers)
    {
        if ( p->RecvPacket( msg ) )
        {
            break;
        }
    }
    _lock_handle.unlock();
}

void UserService::SendPacket( int server_type, BufferPtr buf )
{
    Write( server_type, buf );
}
