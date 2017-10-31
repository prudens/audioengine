#ifndef INTERFACE_SNAIL_AUDIO_ENGINE_H
#define INTERFACE_SNAIL_AUDIO_ENGINE_H
#pragma once

#include "SnailAudioEngineHelper.h"

extern "C"
{
/**
 * @brief     *Must* be called once on main thread(UI thread) before other api on android platform.
 * @return    int.see error code
 * @param     void * ptrJavaContext. should get by getApplicationContext().
 */
int  InitAndroid( void* JavaAppilcationContext );

/**
 * @brief     Init sdk setting, should call before other api(except InitAndroid).
 * @return    int
 * @param     const char * wkfolder. sdk work path.
 * @param     int region. application publish region.detault in china(mainland)
 */
int InitAudioSDK( const char* wkfolder, int region );

/**
 * @brief     Called if all all rooms have left,and don't use the sdk anymore.
 * @return    void
 */
void CleanAudioSDK();

/**
 * @brief     Sdk need a authorization code to get audio service.If null ,will enter test room only.
 * @return    int
 * @param     const char * authokey.Please contact with the sdk service provider.
 */
int  SetAuthoKey( const char* authokey );

/**
 * @brief     Set log level, log information will print on console.
 * @return    int
 * @param     int level.see LogLevel
 */
int  SetLogLevel( int level );

/**
 * @brief     Set language locale,default value is zh_cn
 * @return    int. see error code
 * @param     const char * locale
 *            Here is supported:
              "zh_cn" Simplified Chinese
              "zh_tw" traditional Chinese
              "en_us" English
 */
int  SetLocale( const char* locale = "zh_cn" );

// see SnailAudioEngineHelper.h
const char* GetErrorDescription( int ec );

/**
 * @brief     Get sdk version
 * @return    const char*
 */
const char* GetSDKVersion();
}

namespace audio_engine{
class IAudioRoom;
/**
 * @brief     Create a audio room engine,caller should destroy it using Release function.
 * @return    int error code
 * @param     IAudioRoom** audioRoom. return a audio room instance if ec == ERR_OK.
 */
int CreateAudioRoom( IAudioRoom** audioRoom );

class IAudioRoomEventHandler
{
public:
    /**
     * @brief     Called back when Login() finish.
     * @return    void
     * @param     const char* roomkey 
     *            the roomkey passed to Login().
     * @param     UID uid
     *            the uid passed to Login().
     * @param     int ec see error code.
     */
    virtual void RespondLogin( const char* roomkey, UID uid, int ec ) = 0;

    /**
    * @brief     Called back when Logout() finish.
    * @return    void
    * @param     const char * roomkey. the roomkey passed  to Login().
    * @param     int ec.see error code.
    */
    virtual void RespondLogout( const char* roomkey, UID uid, int ec ) {}

    /**
    * @brief     Called back when SetRoomAttr is called.
    * @return    void
    * @param     const char * name. passed to SetRoomAttr.
    * @param     const char * value.passed to SetRoomAttr.
    * @param     int ec.see error code
    */
    virtual void RespondSetRoomAttr( const char* name, const char* value, int ec ) {}

    /**
    * @brief     Notify the room attribute had changed
    * @return    void
    * @param     const char * name
    * @param     const char * value
    */
    virtual void NotifyRoomAttrChanged( const char* name, const char* value ) {}

    /**
    * @brief     Notify the network is lost.
    * @return    void
    */
    virtual void NotifyConnectionLost() {}

    /**
     * @brief     Called when the network reconnect successfully.
     * @return    void
     * @param     const char * roomkey. the roomkey passed to Login().
     * @param     UID uid
     *            the UID passed to Login().
     */
    virtual void NotifyReConnected( const char* roomkey, UID uid ) {}

    /**
    * @brief     Notify this room is closed.
    * @return    void
    * @param     const char * roomkey.The roomkey passed to Login().
    * @param     int reason. see RoomCloseReason
    */
    virtual void NotifyRoomClose( const char* roomkey, int reason ) {}

    /**
    * @brief     Notify caller had a same uid logined on other devices.
    * @return    void
    */
    virtual void NotifyDuplicateLogined() {}
};

class IAudioRoom
{
public:
    /**
     * @brief     Release the engine resources.After Release called,*must not* call other member function anymore.
     *            WARNING: don't call release(true) in the IAudioRoomEventHandler callbacks.
     * @return    void
     * @param     bool sync
     *            If true, sdk release the engine resources and return after all resources have been destroyed. It maybe *block* for a while.
     *            If false, sdk release the engine resources asynchronous.It always returns immediately.
     */
    virtual void Release( bool sync ) = 0;
    /**
     * @brief     *Must be* called before Login().All event response(or notice) is triggered through handler.
     * @return    void
     * @param     IAudioRoomEventHandler * handler.Caller should inherit the interface IAudioRoomEventHandler,
     *            and overwrite the member function to process the event.
     *            WARNING: The caller must guarantee that the handler remains valid until the Release is called.
     * @param     bool use_poll
     *            If true: caller *must* call Poll *repeatedly* to trigger the event working according to vertical synchronization such as onUpdate(game scene).
     *            In other words, handler member function is always called in the same thread which Poll function called.
     *            If false: handler member function is called in other thread, caller should guarantee the thread safely.
     */
    virtual void RegisterEventHandler( IAudioRoomEventHandler* handler, bool use_poll ) = 0;

    /**
     * @brief     see SnailAudioModule.h
     * @return    IModule** module
     */
    virtual IModule* GetModule( int iid ) = 0;
    /**
     * @brief     Login room asynchronous.Once logined successfully(or failed), IAudioRoomEventHandler::RespondLogin is called.
     * @return    int.see error code.
     * @param     const char * roomkey.a null-terminated byte string.
     * @param     UID uid. a null-terminated byte string
     * @param     const char * user_extend. a null-terminated byte string,can be received by GetUserExtend().
     * @param     bool loop_try_login = true.if true, will always try to login until Logout is called.
     */
    virtual int   Login( const char* roomkey, UID uid, const char* user_extend = NULL, bool loop_try_login = true ) = 0;
    /**
     * @brief     Leave room successfully(or failed),IAudioRoomEventHandler::RespondLogout is called.
     * @return    int see error code.
     */
    virtual int  Logout() = 0;
    /**
     * @brief     Query the login status.
     *            See LOGIN_STATUS if return >=0.
     *            See error code if return < 0.
     */
    virtual int  GetLoginStatus() = 0;

    /**
    * @brief     *option* function. Customize room additional information.
    * @return    int see error code.
    * @param     const char * name.null-terminated byte string.
    * @param     const char * value.null-terminated byte string.
    */
    virtual int  SetRoomAttr( const char* name, const char* value ) = 0;

    /**
    * @brief     Get room additional information passed to SetRoomAttr().
    * @return    int see error code.
    * @param     const char * name.null-terminated byte string.
    * @param     StringPtr & value.a value string,maybe null.
    */
    virtual int  GetRoomAttr( const char* name, StringPtr& value ) = 0;
    /**
     * @brief     Caller should call Poll repeatably(about 30-100ms period),if caller set parameter use_poll = true to SetEventHandler,.
     * @return    void
     */
    virtual void Poll() = 0;
};




}//audio_engine

#endif//INTERFACE_SNAIL_AUDIO_ENGINE_H
