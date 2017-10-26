#ifndef SNAIL_AUDIO_ENGINE_HELPER_H
#define SNAIL_AUDIO_ENGINE_HELPER_H
#pragma once

#if defined _WIN32 || defined __CYGWIN__
    typedef __int64 int64_t;
    typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif
#ifndef NULL
#define NULL 0
#endif

//sdk  error code
#define ERR_OK                             0   // 
#define ERR_GENERIC                       -1   // 
#define ERR_NOT_INITIALIZE                -2   //
#define ERR_INVALID_ARGUMENT              -4
#define ERR_INVALID_AUTHO_KEY             -5
#define ERR_INVALID_ROOM_KEY              -6
#define ERR_INVALID_USER_ID               -7
#define ERR_NOT_SUPPORTED                 -8    // the function is not supported.
#define ERR_NOT_VERSION_SUPPORTED         -9    // the sdk is not supported ,please update sdk.
#define ERR_NOT_LOGINED                   -10   // user is not logined.
#define ERR_SERVER_CONNECT_FAILED         -11
#define ERR_TIME_OUT                      -12   // the async function is called timeout.
#define ERR_ALREADY_IN_USE                -13   // last operator has not finished,can not call it.
#define ERR_USER_NOT_FOUND                -15
#define ERR_MSG_NOT_FOUND                 -16
#define ERR_ROOM_NOT_FOUND                -17

// audio device manager error code
#define ERR_ADM_NO_FOUND_SPEAKER          -201
#define ERR_ADM_NO_FOUND_MICPHONE         -202
#define ERR_ADM_OPEN_SPEAKER_FAILED       -203
#define ERR_ADM_OPEN_MICPHONE_FAILED      -204
#define ERR_ADM_NO_VALID_DATA             -205   // can not record activity sound.
#define ERR_ADM_NO_RECORD_PERMISSION      -206   // need record permission.
#define ERR_ADM_TIME_TOO_SHORT            -207   // record audio message's time is too short to failed.
#define ERR_ADM_TIME_TOO_LONG             -208   // record audio message's time is too long to stop recording.

//network error code
#define ERR_NETWORK_POOL                  -301   // network is not so good.
#define ERR_NETWORK_BROKEN                -302   // network is broken.
#define ERR_NET_FILE_UPLOAD_FAILED        -303   // upload file failed.
#define ERR_NET_FILE_DOWNLOAD_FAILED      -304   // download file failed.

//local file error code
#define ERR_FILE_NOT_FOUND                -401   
#define ERR_FILE_OPEN_FAILED              -402

#define VER_CHINA 0      //China (except Hong Kong,Macao,Taiwan)
#define VER_ASIA  1      //Asia(except China,including Hong Kong,Macao,Taiwan etc.)
#define VER_UROPE 2      //Europe area
#define VER_AMERICA 3    //America area


enum LoginedStatus
{
    LSLogout,     //
    LSLogined,    //
    LSLogining,   //
};

enum LogLevel
{
    LEVEL_CLOSE   = 0,
    LEVEL_VERBOSE = 1,
    LEVEL_DEBUG   = 2,
    LEVEL_INFO    = 3,
    LEVEL_WARN    = 4,
    LEVEL_ERROR   = 5,
};

enum RoomCloseReason
{
    ROOM_IS_EXPIRED = 1,
};

enum eMsgType
{
    msg_text = 1,		 // text message, sdk will not modify the content,advise using utf8 format for cross platform.
    msg_audio = 2,		 // audio message
    msg_binary = 3,      // binary message
    msg_cmd = 10,        // cmd message, server will not cache the type message.
};

enum ModuleID
{
    USER_MODULE = 1,//IUserModule
    REAL_ADUIO_MODULE = 2,//IRealAudioModule
    MESSAGE_MODULE = 3,//IMessageModule
    VOLUME_MODULE = 4,//IVolumeModule
    MIC_ORDER_MODULE = 5,//IMicOrderModule
};

#ifdef __cplusplus
namespace snail{ namespace audio{
template<class T>
class AutoPtr
{
public:
    typedef T value_type;
    typedef T* pointer_type;

    AutoPtr( pointer_type p = NULL )
        :ptr_( p )
    {
    }
    ~AutoPtr()
    {
        if ( ptr_ ) ptr_->release();
    }
    operator bool() const
    {
        return ptr_ != NULL;
    }
    const value_type& operator*( ) const
    {
        return *ptr_;
    }

    const pointer_type operator->( ) const
    {
        return ptr_;
    }
    pointer_type get() const
    {
        return ptr_;
    }
    void reset( pointer_type ptr = 0 )
    {
        if ( ptr != ptr_ && ptr_ )
            ptr_->release();
        ptr_ = ptr;
    }
private:
    AutoPtr( const AutoPtr& );
    AutoPtr& operator=( const AutoPtr& );
private:
    pointer_type ptr_;
};

class String
{
public:
    virtual bool        empty() const = 0;
    virtual const char* c_str() = 0;
    virtual const char* data() = 0;
    virtual size_t      length() = 0;
    virtual void        release() = 0;
    virtual ~String() {}
};
typedef AutoPtr<String> StringPtr;


class Message
{
public:
    virtual int           msgid()const = 0;             // index of message list
    virtual const char*   fromuid()const = 0;            // the remote user of sent message 
    virtual const char*   extends()const = 0;            // extend data
    virtual uint64_t      msgtime()const = 0;            // send message time, in ms
    virtual int           msgtype()const = 0;            // message type (msg_text msg_audio ...)
    virtual const char*   content()const = 0;            // message content
    virtual int           length()const = 0;            // length of content
    virtual ~Message() {}
};

typedef const char* UID;// user id type
class User
{
public:
    virtual UID          userid()const = 0;         // uid
    virtual const char*  extends()const = 0;        // extend data    
    virtual bool         IsDisableSpeak()const = 0; // is disable speak
    virtual bool         IsBlocked()const = 0;      // is block
    virtual const char*  attr( const char* name ) const = 0;
    virtual void         release() = 0;        // can not call it.
    virtual ~User() {}
};
typedef AutoPtr<User> UserPtr;

class UserList
{
public:
    virtual User*  operator[]( size_t idx )const = 0;
    virtual User*  at( size_t idx )const = 0;
    virtual size_t size()const = 0;
    virtual void   release() = 0;  // can not call it.
    virtual ~UserList() {}
};
typedef AutoPtr<UserList> UserListPtr;

class MessageList
{
public:
    virtual Message* operator[]( size_t idx )const = 0;
    virtual Message* at( size_t idx )const = 0;
    virtual size_t   size()const = 0;
    virtual void     release() = 0; // can not call it.
    virtual ~MessageList() {}
};




class IModule
{
	/*inner*/
	
public:
    /**
     * @brief     A brief description for the module
     * @return    const char*
     */
    virtual const char* Description() { return ""; }
    /**
     * @brief     Get module id,see ModuleID.
     * @return    int
     */
    virtual int Id() { return -1; }
    virtual ~IModule() {}
};

}}//snail::audio
#endif
#endif//SNAIL_AUDIO_ENGINE_HELPER_H
