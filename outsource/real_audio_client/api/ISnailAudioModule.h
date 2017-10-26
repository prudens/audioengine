#ifndef SNAIL_AUDIO_MODULE_H
#define SNAIL_AUDIO_MODULE_H
#pragma once
#include "SnailAudioEngineHelper.h"
namespace snail {namespace audio{

class IUserEventHandler
{
public:
    /**
    * @brief     Called back when SetUserAttr() is called.
    * @return    void
    * @param     UID uid
    * @param     const char * name.passed to SetUserAttr().
    * @param     const char * value.passed to SetUserAttr().
    * @param     int ec
    */
    virtual void RespondSetUserAttr( UID uid, const char* name, const char* value, int ec ) {}

    /**
    * @brief     Notify the user attribute has changed
    * @return    void
    * @param     UID uid. the attribute of user.
    * @param     const char * name
    * @param     const char * value
    */
    virtual void NotifyUserAttrChanged( UID uid, const char* name, const char* value ) {}
    /**
    * @brief     Notify a new user joined this room.
    * @return    void
    * @param     UID uid.the UID of the remote user
    */
    virtual void NotifyUserEnterRoom( UID uid ) {}

    /**
    * @brief     Notify a user leave this room.
    * @return    void
    * @param     UID uid
    *            the UID of the remote user
    */
    virtual void NotifyUserLeaveRoom( UID uid ) {}

    /**
    * @brief     Notify the user is speaking now.
    * @return    void
    * @param     UID uid. the UID of the remote user or yourself.
    * @param      int volume. value range[0-100].
    */
    virtual void NotifyUserSpeaking( UID uid, int volume ) {}

    /**
    * @brief     Called back when KickOff() is called.
    * @return    void
    * @param     UID uid. The user id passed to KickOff().
    * @param     int ec. see error code
    */
    virtual void RespondKickOff( UID uid, int ec ) {}

    /**
    * @brief     Notify a user is kickoff. If the user is yourself, the Caller must call Logout().
    * @return    void
    * @param     UID uid
    */
    virtual void NotifyKiceOff( UID uid ) {}
};
class IUserModule :public IModule
{
public:
    
    /**
     * @brief     *Must* be called before login if caller want to enable this module.
     * @return    int
     * @param     IUserEventHandler * handler
     */
    virtual void RegisterEventHandler( IUserEventHandler* handler) = 0;
    /**
    * @brief     *option* function. Customize user additional information.Caller can get it by User*
    * @return    int. see error code.
    * @param     UID uid. the remote or local user id.
    * @param     const char * name. null-terminated byte string.
    * @param     const char * value. null-terminated byte string.
    */
    virtual int SetUserAttr( UID uid, const char* name, const char* value ) = 0;
    /**
    * @brief     Query total number of user in the same room.Caller must call EnablePullUserList(true) before Login().
    * @return    int
    *            Return user count if return >=0.
    *            See error code if return < 0.
    */
    virtual int  GetUserCount() = 0;
    /**
    * @brief     get user list of the room.Caller must call EnablePullUserList(true) before Login().
    * @return    int see error code
    * @param     UserListPtr& userlist.
    */
    virtual int  GetUserList( UserListPtr& userlist ) = 0;

    virtual int  GetUser( UID uid, UserPtr& user ) = 0;
    /**
    * @brief     Kick off the remote user from room.
    * @return    int see error code
    * @param     UID uid.  the remote user.
    */
    virtual int  KickOff( UID uid ) = 0;
};

class IRealAudioEventHandler
{
public:
    /**
    * @brief     Called back when DisableSpeaking() is called.
    * @return    void
    * @param     UID uid.passed to DisableSpeaking().
    * @param     bool disable. passed to DisableSpeaking().
    * @param     int ec see error code
    */
    virtual void RespondDisableSpeaking( UID uid, bool disable, int ec ) {}

    /**
    * @brief     Notify a user disable(enable) speak.
    * @return    void
    * @param     UID uid
    * @param     bool disable
    */
    virtual void NotifyDisableSpeaking( UID uid, bool disable ) {}

    /**
    * @brief     Called back when BlockUser() is called.
    * @return    void
    * @param     UID uid. passed to BlockUser().
    * @param     bool block.passed to BlockUser().
    * @param     int ec
    */
    virtual void RespondBlockUser( UID uid, bool block, int ec ) {}
};
class IRealAudioModule:public IModule
{
public:
    /**
    * @brief     *Must* be called before login if caller want to enable this module.
    * @return    int
    * @param     IRealAudioEventHandler * handler
    */
    virtual void RegisterEventHandler( IRealAudioEventHandler* handler ) = 0;
    /**
    * @brief     Enable local user speak.
    * @return    int. see error code
    * @param     bool enable. If true, local user can speak.
    */
    virtual void EnableSpeak( bool enable ) = 0;
    /**
    * @brief     Query local user if can speak.
    * @return    int
    *            If return > 0, local user can speak.
    *            If return == 0,local user can not speak.
    *            If return < 0, an error occurred,see error code.
    */
    virtual bool  IsEnableSpeak() = 0;
    /**
    * @brief     Enable local user play back.
    * @return    int see error code
    * @param     bool enable. If true,local user can play back.
    */
    virtual void  EnablePlayout( bool enable ) = 0;
    /**
    * @brief     Query local user can play back
    * @return    int
    *            if > 0, local user can play back.
    *            if == 0,local user can not play back.
    *            if < 0, an error occurred,see error code.
    */
    virtual bool IsEnablePlayout() = 0;
    /**
    * @brief     Block a remote user.
    * @return    int. Return 0 if success or an error code.
    * @param     UID uid. the remote user.
    * @param     bool block. If true, sdk will not receive the uid's sound.
    */
    virtual int  BlockUser( UID uid, bool block ) = 0;
    /**
    * @brief     Disable remote user speak access.
    * @return    int .see error code/
    * @param     UID uid.the remote user.
    * @param     bool disspeak.  If true,uid can not speak.
    */
    virtual int  DisableSpeaking( UID uid, bool disspeak ) = 0;
};

class IMessageEventHandler
{
public:
    /**
    * @brief     Called after Login() sucessfully
    * @return    void
    * @param     MessageList * messages. the list of history msg.
    */
    virtual void NotifyRecvHistoryMsgList( const MessageList* messages ) {}
    /**
    * @brief     Notify start recording audio message, response of the StartRecord().
    * @return    void
    */
    virtual void NotifyAudioMsgRecordBegin() {}

    /**
    * @brief     Called when the function StopRecord is called or the record time is too long.
    *            WARNING:an error occurred if call StartRecord(),sdk will report an error code by this function,and url is null.
    * @return    void
    * @param     const char * msg_url. The audio message id that record audio successfully, is NULL if ec != 0.
    * @param     int ec. see error code.
    */
    virtual void NotifyAudioMsgRecordEnd( const char* msg_url, int ec ) {}


    /**
    * @brief     Called if start playing a audio message.
    * @return    void
    * @param     const char * msg_url.the audio message id that record audio message.
    */
    virtual void NotifyAudioMsgPlayBegin( const char* msg_url ) {}


    /**
    * @brief     Called after playing url completely.
    *            WARNING: sdk will stop play url and report a error code by this function,
    *                     when an error occurred(url not found,unsupported format,etc. ).
    * @return    void
    * @param     const char * url. the audio message id that record audio message.
    * @param     int ec.see error code.
    */
    virtual void NotifyAudioMsgPlayEnd( const char* url, int ec ) {}


    /**
    * @brief     Call back when SendMessage is called.
    * @return    void
    * @param     int msg_type.see eMsgType
    * @param     const char * data
    *            if type = text_msg, data is text, text format is utf8;
    *            if type = audio_msg,data is audio message url.
    * @param     int length.the length of data.
    * @param     int ec.see error code.
    */
    virtual void RespondSendMsg( int msg_type, const char* data, int length, const char* extend, UID to_user, int msg_id, int ec ) {}

    /**
    * @brief     notify that receive a new message.
    *            NOTE: you maybe give a new type message, so you should parse it by yourself.
    * @return    void
    * @param     Message* msg.
    */
    virtual void NotifyRecvMsg( const Message* msg ) {}

    /**
    * @brief     Call back when GetHistoryMsgList is called.
    * @return    void
    * @param     int ec.see error code.
    * @param     MessageList * messages.merssage list.
    */
    virtual void RespondGetHitoryMsgList( int ec, const MessageList* messages ) {}

    /**
    * @brief     Called back when SoundToText() is called.
    * @return    void
    * @param     const char * url.audio message id.
    * @param     int ec.see error code.
    */
    virtual void RespondSpeechToText( const char* url, const char* text, int ec ) {}
};
class IMessageModule:public IModule
{
public:

    /**
    * @brief     *Must* be called before login if caller want to enable this module.
    * @return    int.see error code
    * @param     IMessageEventHandler* handler
    */
    virtual void RegisterEventHandler( IMessageEventHandler * handler) = 0;
    /**
    * @brief     Start record a audio message
    * @return    int see error code
    * @param     bool stt_sync
    *            If true,sdk will start a service *Speech Recognition* online synchronously.
    *            This will consume data traffic about 10k/s additional.Caller can call
    *            GetSpeechText to get stt result after complete the recording.
    */
    virtual int  StartRecord( bool stt_sync = false ) = 0;
    /**
    * @brief     Completed recording, caller will recv a event call back,IAudioRoomEventHandler::RespondAudioMsgRecord.
    * @return    void
    * @param     bool cancel
    *            If true,sdk will cancel the recording, and not call IAudioRoomEventHandler::RespondAudioMsgRecord.
    *            If false, sdk will complete the work.
    */
    virtual void  StopRecord( bool cancel = false ) = 0;
    /**
    * @brief     Query sdk if is recording audio message currently
    * @return    int
    *            return > 0 if is recording.
    *            return == 0 if is not recording.
    *            return < 0 if an error occurred,see error code.
    */
    virtual bool IsRecordingAudioMsg() = 0;
    /**
    * @brief     Play back a audio message.If it is playing message currently,sdk will call StopPlayout automatically.
    *            When downloading audio message from url,sdk will call IAudioRoomEventHandler::NotifyAudioMsgDownLoadProgress.
    *            When download completed and start playing back ,sdk will call IAudioRoomEventHandler::NotifyAudioMsgPlayoutStart.
    *            When complete work(or failed), sdk will call IAudioRoomEventHandler::NotifyAudioMsgPlayEnd.
    * @return    int see error code
    * @param     const char * url. null-terminated byte string, a audio message id.
    */
    virtual int  StartPlayout( const char* url ) = 0;
    /**
    * @brief     Terminate playing audio message.
    * @return    void
    */
    virtual void  StopPlayout() = 0;
    /**
    * @brief     Query sdk is playing audio message currently.
    * @return    int
    *            Return 1 if is playing.
    *            Return 0 if is not playing.
    *            Return < 0 if a error occur,see error code.
    */
    virtual bool  IsPlayingAudioMsg() = 0;
    /**
    * @brief     Query a audio message's length of time in seconds.
    * @return    int
    *            Return the seconds of audio time if > 0.
    *            Return an error code if < 0.
    * @param     const char * url. null-terminated byte string, a audio message id.
    */
    virtual int  GetAudioMsgTimeSpan( const char* url ) = 0;
    /**
    * @brief     Query if a audio message is existed in local filesystem.
    *            By default, sdk will not download audio message automatically when received a message.
    * @return    int see error code
    * @param     const char * url. null-terminated byte string, a audio message id.
    */
    virtual bool IsAudioMsgLocalExsit( const char* url ) = 0;
    /**
    * @brief     Send a message,maybe a text message,or a audio message.
    * @return    int. see error code.
    * @param     int msgtype.See eMsgType
    * @param     const char * data
    *            If msgtype == msg_text, data is utf8 string.
    *            If msgtype == msg_audio, data is audio message url.
    *            If msgtype == msg_binary,data is user defined.
    * @param     int length.The length of data,in bytes.
    * @param     const char * extends. Option,user defined.
    * @param     UID to_user.Option,the *specified* remote user that the message want to send.if null, will broadcast to all.
    */
    virtual int  SendMsg( int msgtype, const char* data, int length, const char* extends = NULL, UID to_user = NULL ) = 0;
    /**
    * @brief     Get history message list asynchronous. when complete work, sdk will call IAudioRoomEventHandler::NotifyRecvMessageList
    * @return    int. see error code.
    * @param     uint32_t msg_idx
    *            The message index of message list.see Message::msgidx().caller can use [last idx + 1] to api to get next group messages.
    * @param     int msg_count
    *            The count of message caller want to pull from server,base msg_idx.
    */
    virtual int  GetHistoryMsgList( int msg_idx, int msg_count ) = 0;
    /**
    * @brief     Start a service *Speech Recognition* online synchronously.When compelete work,sdk call IAudioRoomEventHandler::RespondSoundToText
    *            WARNING:it will consume data traffic about 10k/s additional.
    *            STT service is limit once at the same time.
    * @return    int see error code
    * @param     const char * url.  null-terminated byte string, a audio message id.
    */
    virtual int  StartSpeechToText( const char* url ) = 0;
    /**
    * @brief     Stop or cancel stt,stt service is closed immediately.Sdk will call IAudioRoomEventHandler::RespondSoundToText
    * @return    void
    * @param     bool cancel.If true, just cancel the work,not call IAudioRoomEventHandler::RespondSoundToText.
    */
    virtual void  StopSpeechToText( bool cancel ) = 0;
    /**
    * @brief     Query sdk if have a stt service currently.
    * @return    int
    *            Return 1 if is stt now.
    *            Return 0 if is not stt now.
    *            Return < 0 if an error occurred.
    */
    virtual bool IsSpeechToTextNow() = 0;
    /**
    * @brief     Sdk will cache recent stt results in memory.Caller can pass a audio message url to get stt result immediately.
    * @return    int. see error code.
    * @param     const char * url.null-terminated byte string, a audio message id.
    * @param     const char * * text
    *            null-terminated byte string, stt result, utf8 format,maybe null.Caller should not modify the content.
    */
    virtual int  GetTextOfSpeech( const char* msg_url, const char** text ) = 0;
};

class IVolumeModule:public IModule
{
public:
    /**
    * @brief     Adjust the sound volume of the remote user.
    * @return    int.see error code.
    * @param     UID uid. the remote user
    * @param     int volume. valid range:[0,100]
    */
    virtual int AdjustAudioVolume( UID uid, int volume ) = 0;
    /**
    * @brief     Adjust mixer sound volume of the all remote user.
    * @return    int.see error code.
    * @param     int volume. valid range:[0,100]
    */
    virtual int AdjustPlaybackVolume( int volume ) = 0;
    /**
    * @brief     Adjust local user's recording volume.
    * @return    int.see error code
    * @param     int volume. valid range:[0,100]
    */
    virtual int AdjustRecordingVolume( int volume ) = 0;
    /**
    * @brief     query mixed sound volume of the all remote user.
    * @return    int
    *            Return volume if >= 0.
    *            Return an error code.
    */
    virtual int GetPlaybackDeviceVolume() = 0;
};

class IMicOrderEventHandler
{
public:
    /**
     * @brief     Notify all users if AddToMicList called.
     * @return    void
     * @param     UID uid
     */
    virtual void NotifyMicOrderUserJoin( UID uid ) {}
    /**
     * @brief     Called back when AddToMicList called.
     * @return    void
     * @param     UID uid
     * @param     int ec
     */
    virtual void RespondAddToMicList( UID uid, int ec ) {}
    /**
     * @brief     Notify all users if RemoveFromMicList
     * @return    void
     * @param     UID uid
     */
    virtual void NotifyMicOrderUserLeave( UID uid ) {}
    /**
     * @brief     Called back when RemoveFromMicList called.
     * @return    void
     * @param     UID uid
     * @param     int ec
     */
    virtual void RespondRemoveFromMicList( UID uid, int ec ) {}
    /**
     * @brief     Called back when SetMicOrder
     * @return    void
     * @param     UID uid
     * @param     int ec
     */
    virtual void RespondSetMicOrder( UID uid, int ec ) {}
    /**
     * @brief     Notify all users if SetMicOrder called.
     * @return    void
     * @param     UID uid
     */
    virtual void NotifyMicOrderUserChanged( UID uid ) {}
    /**
     * @brief     Should refresh all mic users.
     * @return    void
     */
    virtual void NotifyMicOrderChanged() {}
    /**
     * @brief     Called back when MicOrderClear called.
     * @return    void
     */
    virtual void RespondMicOrderClear() {}
    /**
     * @brief     Notify all users when MicOrderClear called.
     * @return    void
     */
    virtual void NotifyMicOrderClear() {}
};
class IMicOrderModule: public IModule
{
public:
    virtual void RegisterEventHandler( IMicOrderEventHandler* handler ) = 0;
    /**
    * @brief     add a user to mic order list
    * @return    int .see error code
    * @param     UID uid. user id.
    */
    virtual int  AddToMicList( UID uid ) = 0;
    /**
    * @brief     remove the uid from mic order list.
    * @return    int. see error code
    * @param     UID uid.user id
    */
    virtual int  RemoveFromMicList( UID uid ) = 0;
    /**
    * @brief     set uid as a mci order.
    * @return    int.see error code
    * @param     UID uid. user id.
    * @param     int order order level.
    */
    virtual int  SetMicOrder( UID uid, int order ) = 0;
    /**
    * @brief     get uid's mic order.
    * @return    int. return order if >0.
    * @param     UID uid.
    */
    virtual int  GetMicOrder( UID uid ) = 0;
    /**
    * @brief     get mic list count.
    * @return    int.
    */
    virtual int  GetMicUserCount() = 0;
    /**
    * @brief     get mic order list.
    * @return    int. return count if >= 0, else return error code.
    */
    virtual int  GetMicUserList( UserListPtr& userlist ) = 0;

    /**
     * @brief     Get Mic order user, maybe null.
     * @return    int
     * @param     UID uid
     * @param     UserPtr user
     */
    virtual int GetMicUser( UID uid, UserPtr& user ) = 0;
};

}}//snail::audio
#endif//SNAIL_AUDIO_MODULE_H
