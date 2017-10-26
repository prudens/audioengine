#include "client_manager.h"
#include <ctime>
#include <cstdlib>
#include <string>

int static g_mid = 0;
ClientManager::ClientManager(std::string logpath)
{
    g_mid = 0;
    SetAuthoKey( nullptr );
    SetLogLevel( LEVEL_VERBOSE );
    InitAudioSDK(logpath.c_str(),VER_CHINA);
}

ClientManager::~ClientManager()
{
    _audio_clients.clear();
    g_mid = 0;
    CleanAudioSDK();
}

int ClientManager::NewClient( MessageQueue*queue )
{
    std::string strid;

    g_mid++;
    _audio_clients[g_mid] = std::make_shared<AudioClient>(g_mid );
    return g_mid;
}

void ClientManager::DeleteClient( int mid )
{
    _audio_clients.erase( mid );
    if (_audio_clients.empty())
    {
        g_mid = 0; // 重新开始
    }
}

void ClientManager::Login( int mid /*= 1*/, int rid /*= 50*/,std::string uid )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        _audio_clients[mid]->Login( rid,uid );
    }
}

void ClientManager::Logout( int mid )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        _audio_clients[mid]->Logout();
    }
}


void ClientManager::CallFromMainThread(int mid, void* id )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        //_audio_clients[mid]->CallFromMainThread(id);
    }
}

void ClientManager::GetUserList(int mid)
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        _audio_clients[mid]->GetUserList();
    }
}

int ClientManager::GetLoginedStatus( int mid )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return _audio_clients[mid]->GetLoginedStatus();
    }
    else
    {
        return LSLogout;
    }
}

bool ClientManager::IsRecordingMsg(int mid)
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return _audio_clients[mid]->IsRecordingMsg();
    }
    else
    {
        return false;
    }
}

bool ClientManager::IsPlayingMsg(int mid)
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return _audio_clients[mid]->IsPlayingMsg();
    }
    else
    {
        return false;
    }
}

void ClientManager::EnableSpeak(int mid, bool enable )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return _audio_clients[mid]->EnableSpeak( enable );
    }
}

void ClientManager::EnablePlay(int mid, bool enable )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return _audio_clients[mid]->EnablePlay( enable );
    }
}

void ClientManager::RecordMsg( int mid, bool bStop,bool stt )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return _audio_clients[mid]->RecordMsg( bStop,stt );
    }
}

void ClientManager::PlayMsg( int mid, const char* msgid, bool bStop )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return _audio_clients[mid]->PlayMsg( msgid, bStop );
    }
}

void ClientManager::GetMsgList( int mid, int index, int count )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return _audio_clients[mid]->GetMsgList(index,count );
    }
}

void ClientManager::StartSTT(int mid, const char* msgid )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end())
    {
        return it->second->StartSTT(msgid);
    }
}


void ClientManager::CancelSTT( int mid  )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return it->second->CancelSTT( );
    }
}

bool ClientManager::IsSTT( int mid )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return it->second->IsSTT();
    }

    return false;
}

const char* ClientManager::GetUserid(int mid)
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return it->second->GetUserid();
    }
    return "";
}

void ClientManager::TurnOnSpeakingStatus( int mid, bool status,std::string uid )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return it->second->TurnOnSpeakingStatus(status,uid );
    }
}

void ClientManager::SetRoomAttr( int mid, std::string key, std::string value )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return it->second->SetRoomAttr( key, value );
    }
}

void ClientManager::SetUserAttr( int mid, std::string uid,std::string key, std::string value )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return it->second->SetUserAttr(uid, key, value );
    }
}

void ClientManager::GetUserAttr( int mid, std::string uid, std::string key )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return it->second->GetUserAttr( uid, key );
    }
}

void ClientManager::KickOff( int mid, std::string uid )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return it->second->KickOff( uid );
    }
}

void ClientManager::BlockUser( int mid, std::string uid, bool enable )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return it->second->BlockUser( uid, enable );
    }
}

void ClientManager::DisableSpeak( int mid, std::string uid, bool enable )
{
    auto it = _audio_clients.find( mid );
    if ( it != _audio_clients.end() )
    {
        return it->second->DisableSpeak( uid, enable );
    }
}
