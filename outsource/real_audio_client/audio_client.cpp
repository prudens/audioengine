#include <iostream>
#include <cassert>
#include <ctime>
#include "audio_client.h"
#include <functional>
#include "cmd.h"

AudioClient::AudioClient(int mid )
{
    _mid = mid;

    int ec = CreateAudioRoom( &_room );
    _room->RegisterEventHandler( this, false );
     _message    = (IMessageModule*)_room->GetModule( MESSAGE_MODULE );
     _real_audio = (IRealAudioModule*) _room->GetModule( REAL_ADUIO_MODULE );
     _mic_order  = (IMicOrderModule*)  _room->GetModule( MIC_ORDER_MODULE );
     _user       = (IUserModule*)_room->GetModule( USER_MODULE );
     _volume     = (IVolumeModule*)_room->GetModule(VOLUME_MODULE);
    _message->RegisterEventHandler( this );
    _real_audio->RegisterEventHandler( this );
    _mic_order->RegisterEventHandler( this );
    _user->RegisterEventHandler( this );
}

AudioClient::~AudioClient()
{
    _room->Logout();
    _room->Release( true );
}

void AudioClient::Login( int roomid,std::string uid )
{
    if ( uid.empty() )
    {
        std::srand( (unsigned int)std::time( 0 ) ); // use current time as seed for random generator
        int random_variable = std::rand();
        uid = std::to_string( random_variable );
    }
    _userid = uid;
   _room->Login( std::to_string( roomid ).c_str(), _userid.c_str() );
}

void AudioClient::Logout()
{
    _room->Logout();
}

void AudioClient::GetUserList()
{
    logprint( "\n%.20s\t%.20s\t%.20s\t%.20s\n\n", "userid", "extend", "禁言", "blocked" );
    UserListPtr userlist;
    if ( _user->GetUserList( userlist ) == 0 )
    {
        int count = userlist->size();
        for ( int i = 0; i < count; i++ )
        {
            auto user = ( *userlist )[i];
            auto ext = user->extends();
            logprint( "%s\n", ext );
            logprint( "%d. %.20s\t%.20s\t%.20s\t%.20s\n\n", i, user->userid(), "", user->IsDisableSpeak() ? "是" : "否", user->IsBlocked() ? "是" : "否" );
        }
        logprint( "房间共有：%d个用户\n", count );
    }
}

int AudioClient::GetLoginedStatus( )
{
    return _room->GetLoginStatus();
}

bool AudioClient::IsRecordingMsg()
{
    return _message->IsRecordingAudioMsg();
}

bool AudioClient::IsPlayingMsg()
{
    int ret = _message->IsPlayingAudioMsg();
    return ret > 0;
}

void AudioClient::EnableSpeak( bool enable )
{
    _real_audio->EnableSpeak( enable );
}

void AudioClient::EnablePlay( bool enable )
{
    _real_audio->EnablePlayout( enable );
}


void AudioClient::RecordMsg(bool bStop, bool stt)
{
    if (bStop)
    {
        _message->StopRecord();
    }
    else
    {
        _message->StartRecord( stt );
    }
}

void AudioClient::PlayMsg( const char* msgid, bool bStop )
{
    if (bStop)
    {
        _message->StopPlayout();
    }
    else
    {
        _message->StartPlayout( msgid );
    }
}

void AudioClient::GetMsgList( int index, int count )
{
    _message->GetHistoryMsgList( index, count );
}

void AudioClient::StartSTT( const char* msgid )
{
    _message->StartSpeechToText( msgid );
}


void AudioClient::CancelSTT()
{
    _message->StopSpeechToText( false );
}

bool AudioClient::IsSTT()
{
    return _message->IsSpeechToTextNow();
}

const char* AudioClient::GetUserid()
{
    return _userid.c_str();
}

void AudioClient::TurnOnSpeakingStatus( bool status,std::string uid )
{
    _turn_on_speak = status;
    _turn_on_speak_uid = uid;
}


void AudioClient::SetRoomAttr( std::string key, std::string value )
{
    _room->SetRoomAttr( key.c_str(), value.c_str() );
}

void AudioClient::SetUserAttr( std::string uid, std::string key, std::string value )
{
    _user->SetUserAttr( uid.c_str(), key.c_str(), value.c_str() );
}

//////////////////////////处理handler 异步事件///////////////////
void AudioClient::RespondLogin( const char* roomkey, UID uid, int ec )
{
    if ( ec == 0)
    {
        logprint( "[%d] 登陆成功。 房间id:%s,用户id:%s\n",_mid, roomkey, uid  );
    }
    else
    {
        logprint( "[%d]登陆失败, 房间id:%s,用户id:%s,错误码:%d\n", _mid, roomkey, uid, ec );
    }
}

void AudioClient::NotifyReConnected( const char* roomkey, UID uid )
{
    logprint("[%d]重新登陆到房间\n",_mid);
}

void AudioClient::RespondLogout( const char* roomkey, UID uid, int ec )
{
    if ( ec == 0 )
    {
        logprint( "[%d] 登出房间成功. 房间id:%s,用户id:%s\n", _mid, roomkey, uid );
    }
    else
    {
        logprint( "[%d]等出房间失败。房间id:%s,用户id:%s,错误码:%d\n", _mid, roomkey, uid, ec );
    }
}

void AudioClient::NotifyUserEnterRoom( UID uid )
{
    logprint( "[%d]用户（%s）上线\n", _mid,uid );
}

void AudioClient::NotifyUserLeaveRoom( UID uid )
{
    logprint( "[%d]用户（%s）下线\n", _mid, uid );
}


void AudioClient::NotifyUserSpeaking( UID uid, int volume )
{
    if ( _turn_on_speak )
    {
        logprint( "[%d]用户(%s) 正在讲话(%d)...\n", _mid, uid, volume );
    }

}

void AudioClient::NotifyRoomClose( const char* roomkey, int reason )
{
    logprint( "[%d]房间（%s）关闭,原因是：%d\n",_mid,roomkey,reason );
}

void AudioClient::NotifyAudioMsgRecordEnd( const char* url, int ec )
{
    if (ec == 0)
    {
        std::string strEvent = "录音成功 msgid:";
        strEvent += url;
        _message->SendMsg( 2, (char*)url, strlen( (char*)url ), nullptr, nullptr );
        const char* text = nullptr;
        ec = _message->GetTextOfSpeech( url,&text );
        if ( text )
//            logprint( "[%d]转文本结果:%s\n", _mid, snail::tools::UTF8ToGBK( text ).c_str() );

            logprint( "[%d]%s\n", _mid, strEvent.c_str() );
    }
    else
    {
        logprint( "[%d]录音失败，错误码：%d\n",_mid, ec );
    }
}

void AudioClient::NotifyRecvMsg( const Message* msg )
{
    std::string strEvent = "收到语音消息：";
    if ( msg )
    {
        strEvent += msg->content();
        if ( msg->extends() && strlen( msg->extends() ) )
        {
            strEvent += "\n扩展信息：";
            strEvent += msg->extends();
        }
    }
    logprint("[%d]%s\n",_mid, strEvent.c_str() );
}

void AudioClient::RespondGetHitoryMsgList( int ec, const MessageList* msglist )
{
    int count = msglist->size();
    std::string strEvent = "收到一组语音消息(";
    strEvent += std::to_string( count ) + "条)\n";
    strEvent += "\t类型\t\t内容\t\t\n";
    for ( int i = 0; i < count; i++ )
    {
        Message* msg = (*msglist)[i];
        std::string strMsgType;

        if ( msg->msgtype() == msg_text )
        {
            strMsgType = "文本消息";
        }
        else
        {
            strMsgType = "语音消息";
        }

		std::string content = msg->content();// snail::tools::UTF8ToGBK(msg->content());
        strEvent = strEvent + std::to_string( msg->msgid() ) + "\t" + strMsgType + "\t" + content + "\t\n";
    }
    logprint("[%d]%s\n",_mid,strEvent.c_str());
}

void AudioClient::RespondSendMsg( int msg_type, const char* data, int length, const char* extend, UID to_user, int msg_id, int ec )
{
    if ( ec == 0)
    {
        logprint( "[%d]消息发送成功\n", _mid );
    }
    else
    {
        logprint( "[%d]消息发送失败,错误码：%d\n",_mid,ec );
    }
}


void AudioClient::NotifyConnectionLost()
{
    logprint( "[%d]网络不通\n",_mid );
}

void AudioClient::NotifyRoomAttrChanged( const char* name, const char* value )
{
    logprint( "[%d]房间属性变化：name:%s,value:%s\n", _mid,name, value );
}

void AudioClient::NotifyUserAttrChanged( UID uid, const char* name, const char* value )
{
    logprint( "[%d]用户(%s)属性变化：name:%s,value:%s\n",_mid,uid,name,value );
}

void AudioClient::NotifyAudioMsgRecordBegin()
{
    logprint( "[%d]开始录音中...\n",_mid );
}

void AudioClient::NotifyAudioMsgPlayBegin( const char* url )
{
    logprint( "[%d]语音消息（%s）开始播放中...\n", _mid, url );
}

void AudioClient::NotifyAudioMsgPlayEnd( const char* url, int ec )
{
    logprint( "[%d]语音消息（%s）播放完毕,错误码：%d\n",_mid,url,ec );
}

void AudioClient::RespondSpeechToText( const char* url, const char* text, int ec )
{
    if ( ec == 0 )
    {
//        logprint( "[%d]语音转文本成功。url:%s，转换结果：%s\n", _mid, url, snail::tools::UTF8ToGBK( text ).c_str() );
    }
    else
    {
        logprint( "[%d]语音转文本失败,错误码：%d\n",_mid, ec );
    }
}

void AudioClient::RespondKickOff( UID uid, int ec )
{
    if (ec == 0)
    {
        logprint( "[%d]踢出用户（%s）成功\n",_mid,uid );
    }
    else
    {
        logprint( "[%d]踢出用户（%s）失败,错误码：%d\n", _mid, uid, ec );
    }
}

void AudioClient::NotifyKiceOff( UID uid )
{
    logprint( "[%d]用户（%s）被踢出房间\n",_mid,uid );
}

void AudioClient::NotifyDuplicateLogined()
{
    logprint( "[%d]账号在其他地方登陆\n",_mid );
}

void AudioClient::RespondDisableSpeaking( UID uid, bool disable, int ec )
{
    if ( ec == 0 )
    {
        if ( disable )
        {
            logprint( "[%d]禁言用户（%s）成功\n", _mid, uid );
        }
        else
        {
            logprint( "[%d]取消禁言用户（%s）成功\n", _mid, uid );
        }
    }
    else
    {
        if ( disable )
        {
            logprint( "[%d]禁言用户（%s）失败,错误码：%d\n", _mid, uid,ec );
        }
        else
        {
            logprint( "[%d]取消禁言用户（%s）失败,错误码：%d\n", _mid, uid,ec );
        }
    }
}

void AudioClient::NotifyDisableSpeaking( UID uid, bool disable )
{
    if (disable)
    {
        logprint( "[%d]用户(%s)被禁言\n",_mid,uid );
    }
    else
    {
        logprint( "[%d]用户(%s)被解除禁言\n",_mid,uid );
    }
}

void AudioClient::RespondSetRoomAttr( const char* name, const char* value, int ec )
{
    if ( ec == 0)
    {
        logprint( "[%d]设置房间属性(name:%s,value:%s)成功\n",_mid,name,value );
    }
    else
    {
        logprint( "[%d]设置房间属性(name:%s,value:%s)失败，错误码：%d\n", _mid,name,value,ec );
    }

}

void AudioClient::RespondSetUserAttr( UID uid, const char* name, const char* value, int ec )
{
    if ( ec ==0 )
    {
        logprint( "[%d]设置用户（%s）属性(name:%s,value:%s)成功\n",_mid,uid,name,value );
    }
    else
    {
        logprint( "[%d]设置用户（%s）属性(name:%s,value:%s)失败，错误码：%d\n", _mid, uid, name, value,ec );
    }
}

void AudioClient::RespondBlockUser( UID uid, bool block, int ec )
{
    if ( ec == 0 )
    {
        if (block )
        {
            logprint( "[%d]拒听用户（%s）成功\n", _mid, uid );
        }
        else
        {
            logprint( "[%d]取消拒听用户（%s）成功\n", _mid, uid );
        }
    }
    else
    {
        if (block)
        {
            logprint( "[%d]拒听用户（%s）失败，错误码：%d\n", _mid, uid,ec );
        }
        else
        {
            logprint( "[%d]取消拒听用户（%s）失败，错误码：%d\n", _mid, uid, ec );
        }

    }
}

void AudioClient::GetUserAttr( std::string uid, std::string key )
{
    if ( key.empty())
    {
        key = "ext";
    }
    UserPtr ptr;
    _user->GetUser( uid.c_str(),ptr );
    if (ptr)
    {
        auto str = ptr->attr( key.c_str() );
        if (str)
        {
            logprint( "[%d]%s\n", _mid, str );
        }
    }
}

void AudioClient::GetRoomAttr( std::string key )
{
    StringPtr ptr;
    _room->GetRoomAttr( key.c_str(), ptr );
    if (ptr)
    {
        logprint( "[%d]%s\n", ptr->c_str() );
    }
}

void AudioClient::KickOff( std::string uid )
{
    _user->KickOff( uid.c_str() );
}

void AudioClient::BlockUser( std::string uid, bool enable )
{
    _real_audio->BlockUser( uid.c_str(), enable );
}

void AudioClient::DisableSpeak( std::string uid, bool enable )
{
    _real_audio->DisableSpeaking( uid.c_str(), enable );
}


