#include "user_manager.h"

#include "server_module.h"
#include "server_config.h"
#include "base/common_defines.h"
#include "user.h"

UserManager::UserManager()
    :_packet(nullptr)
{
    _task = ServerModule::GetInstance()->GetAsyncTask();
}

UserManager::~UserManager()
{
    
}

void UserManager::Start()
{
    std::string ip;
    int16_t port;
    if ( ServerModule::GetInstance()->GetServerCnfig()->GetServer(1, ip, port) )
    {
		TcpFactory *f = ServerModule::GetInstance()->GetSocketManager();
		_acceptor = f->CreateTcpAcceptr(ip, port);
		ASSERT(_acceptor);
		_acceptor->AsyncAccept( std::bind( &UserManager::HandleAccept, this, std::placeholders::_1, std::placeholders::_2 ) );
	}
}

void UserManager::Stop()
{
    _stop = true;
    _lock.lock();
    for (auto & u: _users )
    {
        u->DettachTcp();
    }
    _lock.unlock();
}

void UserManager::HandleLogin( std::shared_ptr<User> user)
{
    _lock.lock();
    _users.push_back( user );
    _lock.unlock();
    auto pb = std::make_shared<audio_engine::RAUserMessage>();
    auto login_ntf = pb->mutable_notify_login();
    login_ntf->set_state( static_cast<audio_engine::USER_STATE>(user->state()) );
    login_ntf->set_userid( user->userid() );
    login_ntf->set_token( user->token() );
    login_ntf->set_extend( user->extend() );
    login_ntf->set_devtype( static_cast<audio_engine::DEVICE_TYPE>( user->device_type()) );
    BufferPtr buf = _packet.Build( pb );
    if ( buf )
    {
        _lock.lock();
        for ( auto u : _users )
        {
            if ( u != user )
            {
                u->Send( 1, buf );
            }
        }
        _lock.unlock();
    }
}

void UserManager::HandleLogout( std::shared_ptr<User> user )
{
    _lock.lock();
    if (std::find(_users.begin(),_users.end(),user) == _users.end() )
    {
        _lock.unlock();
        return;
    }
    _users.remove( user );
    _lock.unlock();
    auto pb2 = std::make_shared<audio_engine::RAUserMessage>();
    auto login_ntf = pb2->mutable_notify_logout();
    login_ntf->set_token(user->token());
    BufferPtr buf = _packet.Build( pb2 );
    if ( buf )
    {
        _lock.lock();
        for ( auto u : _users )
        {
            if ( u != user )
            {
                u->Send( 1, buf );
            }
        }
        _lock.unlock();
    }
}

void UserManager::UpdateUserExtend(std::shared_ptr<User> user, std::shared_ptr< audio_engine::RAUserMessage> pb)
{
	auto update_extend = pb->update_user_extend();
	update_extend.set_error_code(0);
	BufferPtr buf = _packet.Build(pb);
	_lock.lock();
	for (auto u:_users)
	{
		u->Send(1,buf);
	}
	_lock.unlock();
}

void UserManager::UpdateUserState(std::shared_ptr<User> user, std::shared_ptr< audio_engine::RAUserMessage> pb)
{
	auto update_state = pb->update_user_state();
	update_state.set_error_code(0);
	BufferPtr buf = _packet.Build(pb);
	_lock.lock();
	for (auto u : _users)
	{
		if (u == user)
		{
			continue;
		}
		if (u->token() == update_state.dst_token())
		{
			u->set_state(update_state.state());
		}
		user->Send(1, buf);
	}
	_lock.unlock();
}

bool UserManager::HandleAccept( std::error_code ec, TcpSocketPtr tcp )
{
    if (ec )
    {
        printf("accept error:%s\n",ec.message().c_str());
    }
    if ( !ec)
    {
        std::string ip;
        int16_t port;
        if (!tcp->QuerySocketInfo(ip, port))
        {
            printf( "收到客户端新连接：%s:%u\n",ip.c_str(), (uint16_t)port );
        }
        auto user = std::make_shared<User>(this);
        user->AttachTcp( tcp );
    }
    if ( _stop )
    {
        return false;
    }
	_acceptor->AsyncAccept(std::bind(&UserManager::HandleAccept, this, std::placeholders::_1, std::placeholders::_2));
    return true;
}