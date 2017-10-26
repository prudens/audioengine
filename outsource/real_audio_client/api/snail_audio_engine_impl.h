#pragma once
#include <string>
#include <thread>
#include <list>
#include <memory>
#include <mutex>
#include <map>
#include "../master_control.h"
#include "ISnailAudioModule.h"
#include "ISnailAudioEngine.h"

namespace snail{
namespace audio{

int  TransformErrorCode( int ec );

typedef std::function<void()> EventHandle;

class MessageModuleImpl;
class RealAudioModuleImpl;
class MicOrderModuleImpl;
class VolumeModuleImpl;
class UserModuleImpl; 

class IModuleHandler
{
public:
	virtual EventHandle EventNotifyHandler(int eventid, const void* param){ return nullptr;}
};


class AudioRoomImpl : public IAudioRoom,public IAsyncEventHandler
{ 
	typedef std::shared_ptr<IModule> moduleptr;
public:
	AudioRoomImpl();
	virtual ~AudioRoomImpl();
public:// IAudioRoom
	virtual void      Release(bool sync = true);
	virtual void      RegisterEventHandler(IAudioRoomEventHandler* handler, bool use_poll);
	virtual IModule*  GetModule(int id);
	virtual int       SetRoomAttr(const char* name, const char* value);
	virtual int       GetRoomAttr(const char* name, StringPtr& extend);
    virtual int       Login( const char* roomkey, UID uid, const char* user_extend = NULL, bool loop_try_login = true);
	virtual int       Logout();
	virtual int       GetLoginStatus();
	virtual void      Poll();
public:
    void EnableRealAudio( bool enable );
    void EnableAudioMessage( bool enable );
    void EnablePullUserList( bool enable = false );
	void RecvAsyncEvent(EventHandle handle);
public:
	virtual void RespondLogin(std::string roomkey, std::string uid, int ec);
	virtual void RespondLogout(std::string roomkey, std::string uid, int ec);
private: 
	IModule*  CreateModule(int id);
	void	  ConsumeAllEvent();
	void      StartThreadLoop();
	void      StopThreadLoop();
	void      EventNotifyHandler(int eventid, const void* param);
    int       DoLogin();

public:
	MasterControl _master_control;
	std::map<int, moduleptr> _modules; 
	IAudioRoomEventHandler* _handler = nullptr;
	bool _run_flag = false;
	bool _use_poll = false;
	bool _enable_pull_user_list = false;
	std::thread _async_event_process_thread;
	std::mutex             _event_handle_mutex;
	std::list<EventHandle> _event_handle_list;
	std::string _roomkey;
	std::string _uid;
    std::string _user_extend;
    bool _try_login = false;
    bool _try_logout = false;
    bool _loop_try_login = true;
};

class UserModuleImpl :public IUserModule,public IModuleHandler
{
public:
    UserModuleImpl( AudioRoomImpl*host );
    virtual void RegisterEventHandler( IUserEventHandler* handler );
    virtual int  SetUserAttr( UID uid, const char* name, const char* value );
    virtual int  GetUserCount();
    virtual int  GetUserList( UserListPtr& userlist );
    virtual int  GetUser( UID uid, UserPtr& user );
    virtual int  KickOff( UID uid );
public:
	void*       Handler() { return (IModuleHandler*)this; }
	EventHandle EventNotifyHandler(int eventid, const void* param);
private:
    AudioRoomImpl* _host = nullptr;
    IUserEventHandler* _handler = nullptr;
//    snail::client::media::imedia_base_client* _media_base_client;
};

class MessageModuleImpl : public IMessageModule, public IModuleHandler
{
public:
    MessageModuleImpl( AudioRoomImpl* host );
    ~MessageModuleImpl();
public://IAudioMessageEngine
    virtual void RegisterEventHandler( IMessageEventHandler * handler );
    virtual int  StartRecord( bool stt_sync = false )override;
    virtual void StopRecord( bool cancel = false )override;
    virtual bool IsRecordingAudioMsg()override;
    virtual int  StartPlayout( const char* url )override;
    virtual void StopPlayout()override;
    virtual bool IsPlayingAudioMsg()override;
    virtual int  GetAudioMsgTimeSpan( const char* url )override;
    virtual bool IsAudioMsgLocalExsit( const char* url )override;
    virtual int  SendMsg( int msgtype, const char* data, int length, const char* extends, UID to_user )override;
    virtual int  GetHistoryMsgList( int msg_idx, int msg_count )override;
    virtual int  StartSpeechToText( const char* url )override;
    virtual void StopSpeechToText( bool cancel )override;
    virtual bool IsSpeechToTextNow()override;
    virtual int  GetTextOfSpeech( const char* url, const char** text )override;
protected:
    EventHandle EventNotifyHandler( int eventid, const void* param );
private:
    AudioRoomImpl* _host = nullptr;
    IMessageEventHandler* _handler = nullptr;
   // snail::client::media::imedia_base_client* _media_base_client;

};

class RealAudioModuleImpl :public IRealAudioModule,public IModuleHandler
{
public:
    RealAudioModuleImpl( AudioRoomImpl* host );
    ~RealAudioModuleImpl();
    virtual void RegisterEventHandler( IRealAudioEventHandler* handler );
    virtual int  BlockUser( UID uid, bool block );
    virtual int  DisableSpeaking( UID uid, bool disspeak );
    virtual void EnableSpeak( bool enable );
    virtual bool IsEnableSpeak();
    virtual void EnablePlayout( bool enable );
    virtual bool IsEnablePlayout();
protected:
	EventHandle EventNotifyHandler(int eventid, const void* param);
private:
    AudioRoomImpl* _host = nullptr;
    IRealAudioEventHandler* _handler = nullptr;
   // snail::client::media::imedia_base_client* _media_base_client;
};

class MicOrderModuleImpl :public IMicOrderModule,public IModuleHandler
{
public:
    MicOrderModuleImpl( AudioRoomImpl* host );
    ~MicOrderModuleImpl();
    virtual void RegisterEventHandler( IMicOrderEventHandler* handler );
    /**
    * @brief     add a user to microphone order list
    * @return    int .see error code
    * @param     UID uid. user id.
    */
    virtual int AddToMicList( UID uid );
    /**
    * @brief     remove the uid from microphone order list.
    * @return    int. see error code
    * @param     UID uid.user id
    */
    virtual int RemoveFromMicList( UID uid );
    /**
    * @brief     set uid as a microphone order.
    * @return    int.see error code
    * @param     UID uid. user id.
    * @param     int order order level.
    */
    virtual int SetMicOrder( UID uid, int order );
    /**
    * @brief     get uid's microphone order.
    * @return    int. return order if >0.
    * @param     UID uid.
    */
    virtual int GetMicOrder( UID uid );
    /**
    * @brief     get mic list count.
    * @return    int.
    */
    virtual int GetMicUserCount();
    /**
    * @brief     get mic order list.
    * @return    int. return count if > 0, else return error code.
    */
    virtual int GetMicUserList( UserListPtr& userlist );

    virtual int GetMicUser( UID uid, UserPtr& user );
protected:
	EventHandle EventNotifyHandler(int eventid, const void* param);

private:
    AudioRoomImpl* _host = nullptr;
    IMicOrderEventHandler* _handler = nullptr;
   // snail::client::media::imedia_base_client* _media_base_client;
};

class VolumeModuleImpl :public IVolumeModule
{
public:
    VolumeModuleImpl( AudioRoomImpl* host );
    ~VolumeModuleImpl();
    /**
    * @brief     Adjust the sound volume of the remote user.
    * @return    int.see error code.
    * @param     UID uid. the remote user
    * @param     int volume. valid range:[0,100]
    */
    virtual int AdjustAudioVolume( UID uid, int volume );
    /**
    * @brief     Adjust mixer sound volume of the all remote user.
    * @return    int.see error code.
    * @param     int volume. valid range:[0,100]
    */
    virtual int AdjustPlaybackVolume( int volume );
    /**
    * @brief     Adjust local user's recording volume.
    * @return    int.see error code
    * @param     int volume. valid range:[0,100]
    */
    virtual int AdjustRecordingVolume( int volume );
    /**
    * @brief     query mixed sound volume of the all remote user.
    * @return    int
    *            Return volume if >= 0.
    *            Return an error code.
    */
    virtual int GetPlaybackDeviceVolume();

protected:
	void*       Handler() { return nullptr; } 

public:
    AudioRoomImpl* _host = nullptr;
    IAudioRoomEventHandler* _handler = nullptr;
   // snail::client::media::imedia_base_client* _media_base_client;
};
}}
