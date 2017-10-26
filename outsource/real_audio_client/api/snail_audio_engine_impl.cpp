
#include <memory>
//#include "system/system.h"
#include "snail_audio_engine_impl.h"
#include "module_debug_impl.h"
#include "audio_typedef.h" 

#include"../client_module.h"

#define STR(s) s?s:""

//    using namespace snail::client::media;
    using namespace snail::audio;
    SDKLog Log;
    AudioSDKCfg g_sdk_cfg;
extern "C"{
    int InitAndroid( void* JavaAppilcationContext )
    {
        Log.v( "InitAndroid(%p)\n", JavaAppilcationContext );
        if ( !JavaAppilcationContext )
        {
            Log.e( "InitAndroid 参数错误，JavaAppilcationContext不允许为空\n" );
            return ERR_INVALID_ARGUMENT;
        }
        g_sdk_cfg._init_android = true;
        return ERR_OK;
    }

    int InitAudioSDK( const char* wkfolder, int region )
    {
        Log.v( "InitAudioSDK(%s,%d)\n", STR( wkfolder ), region );
        if ( !wkfolder )
        {
            Log.e( "InitAudioSDK失败, 参数错误： wkfolder不允许为空\n" );
            return ERR_INVALID_ARGUMENT;
        }
        g_sdk_cfg._cfg_path = wkfolder;
        g_sdk_cfg._set_cfg_folder = true;

        if ( region < VER_CHINA || region > VER_AMERICA )
        {
            region = VER_CHINA;
            Log.w( "SetRegion(%d) 调用失败，无效参数\n", region );
        }
        g_sdk_cfg._regionver = region;
		ClientModule::CreateInstance();
        g_sdk_cfg._init = true;
        return ERR_OK;
    }

    void CleanAudioSDK()
    {
        Log.v( "CleanAudioSDK()\n" );
		ClientModule::DestroyInstance();
        g_sdk_cfg._init = false;
    }

    int SetAuthoKey( const char* authokey )
    {
        Log.v( "SetAuthoKey(%s)\n",STR(authokey) );
        if ( !authokey )
        {
            Log.w( "SetAuthoKey 参数为空：只对测试房间有效\n" );
        }
        g_sdk_cfg._set_autho_key = true;
        return ERR_OK;
    }

    int SetLogLevel( int level )
    {
        Log.v( "SetLogLevel(%d)\n", level );
        int inter_level = level / 10000;
        int api_level = level % 10000;
        if ( inter_level < 0 || inter_level > 6 )
        {
            Log.e("SetLogLevel 参数错误：level值无效\n");
            return ERR_INVALID_ARGUMENT;
        }
        Log.setLevel( api_level );
        if (inter_level >= 0 && inter_level < 3)
        {
            g_sdk_cfg._log_level = inter_level;
        }
        return ERR_OK;
    }

    int SetLocale( const char* locale )
    {
        Log.v( "SetLocale(%s)\n", STR(locale) );
        if ( !locale )
        {
            Log.e( "SetLocale 参数错误：locale不允许为空\n" );
            return ERR_INVALID_ARGUMENT;
        }
        if ( strcmp( locale, "zh_cn" ) != 0 &&
             strcmp( locale, "en_us" ) != 0 &&
             strcmp( locale, "zh_tw" ) != 0 )
        {
            Log.e( "SetLocale(%s) 参数错误：当前只支持zh_cn|en_us|zh_tw三种参数\n", locale );
            return ERR_INVALID_ARGUMENT;
        }
        g_sdk_cfg._cfg_locale = locale;
        //SetAudioLocale( locale );
        return ERR_OK;
    }

    const char* GetErrorDescription( int ec )
    {
        Log.v("GetErrorDescription(%d)",ec);
        if ( g_sdk_cfg._cfg_locale == "zh_cn" )
        {
            for ( int i = 0; i < ERROR_CODE_COUNT; i++ )
            {
                if ( g_error_code_zh_cn[i].ec == ec )
                {
                    return g_error_code_zh_cn[i].desc;
                }
            }
            Log.e( "GetErrorDescription(%d) 无法识别的错误码\n", ec );
            return g_error_code_zh_cn[1].desc;
        }
        else
        {
            for ( int i = 0; i < ERROR_CODE_COUNT; i++ )
            {
                if ( g_error_code_en_us[i].ec == ec )
                {
                    return g_error_code_en_us[i].desc;
                }
            }
            Log.e( "GetErrorDescription(%d) invalid error code\n", ec );
            return g_error_code_en_us[1].desc;
        }
    }

    const char* GetSDKVersion()
    {
        Log.v( "GetVersion()\n" );
        return "3.0.1";
    }
}
namespace snail{namespace audio{
    int CreateAudioRoom( IAudioRoom** audioRoom )
    {
        Log.v( "CreateAudioRoom(%p)\n", audioRoom );
        if ( !g_sdk_cfg._init )
        {
            Log.e( "CreateAudioRoom失败，请先调用 InitAudioSDK()\n" );
            return ERR_NOT_INITIALIZE;
        }
        *audioRoom = new AudioRoomImpl();
        return ERR_OK;
    }

    AudioRoomImpl::AudioRoomImpl()
    {
        _run_flag = true;
//        _media_base_client = snail::client::media::CreateMediaBaseClient();
//        _media_base_client->IO_RegisterNotify( std::bind( &AudioRoomImpl::EventNotifyHandler,this,std::placeholders::_1,std::placeholders::_2) ); 
		EnableAudioMessage(false);
		EnablePullUserList(false);
		EnableRealAudio(false);
    }

    AudioRoomImpl::~AudioRoomImpl()
    {
    }

    void AudioRoomImpl::Release( bool sync /*= true */ )
    {
        Log.v("IAudioRoom::Release(%d)\n",sync);
        _run_flag = false;
        if ( sync )
        {
            Log.i("IAudioRoom::Release 使用同步方式，等待sdk释放资源\n");
            StopThreadLoop();
            delete this;
        }
        else
        {
            Log.i( "IAudioRoom::Release 使用异步方式，立即返回，切换到后台释放资源\n" );
            std::thread( [=] () mutable
            {
                StopThreadLoop();
                delete this;
            } ).detach();
        }
    }
    void AudioRoomImpl::RegisterEventHandler( IAudioRoomEventHandler* handler, bool use_poll )
    {
        Log.v( "IAudioRoom::RegisterEventHandler( %p,%d )\n",handler, use_poll );
        if (use_poll)
        {
            Log.i("IAudioRoom::RegisterEventHandler 使用轮询模式来取消息事件，上层要定时（30-100ms左右）调用Poll函数\n");
        }
        if (!handler)
        {
            Log.e( "IAudioRoom::RegisterEventHandler 参数错误：handler不允许为空\n" );
        }
        _handler = handler;
        _use_poll = use_poll;
    }

    IModule* AudioRoomImpl::GetModule( int id )
    {
        Log.v("IAudioRoom::GetModule(%d)\n",id);
		auto it = _modules.find(id);
		if (it == _modules.end()) 
			return CreateModule(id); 
			
		return it->second.get(); 
    }

	IModule* AudioRoomImpl::CreateModule(int id)
	{
		switch (id)
		{
		case USER_MODULE:
		{
			moduleptr md = std::make_shared<UserModuleImpl>(this);
			_modules[id] = md;
			return md.get();
		}
		case REAL_ADUIO_MODULE:
		{
			moduleptr md = std::make_shared<RealAudioModuleImpl>(this);
			_modules[id] = md;
			return md.get();
		}
		case MESSAGE_MODULE:
		{
			moduleptr md = std::make_shared<MessageModuleImpl>(this);
			_modules[id] = md;
			return md.get();
		}
		case VOLUME_MODULE:
		{
			moduleptr md = std::make_shared<VolumeModuleImpl>(this);
			_modules[id] = md;
			return md.get();
		}
		case MIC_ORDER_MODULE:
		{
			moduleptr md = std::make_shared<MicOrderModuleImpl>(this);
			_modules[id] = md;
			return md.get();
		}
		case INNER_MODULE_DEBUGGER:
		{
			moduleptr md = std::make_shared<ModuleDebugerImpl>(this);
			_modules[id] = md;
			return md.get();
		} 
		default:
			break;
		} 
        Log.w( "IAudioRoom::CreateModule 参数错误：找不到对应的模块\n");
		return nullptr;
	}

    int AudioRoomImpl::SetRoomAttr( const char* name, const char* value )
    {
        Log.v( "IAudioRoom::SetRoomAttr(%s,%s)\n", STR(name ),STR(value));
        if (!name)
        {
            Log.e( "IAudioRoom::SetRoomAttr 参数错误：name不允许为空\n" );
            return ERR_INVALID_ARGUMENT;
        }
        std::string strname = STR(name);
        std::string strvalue = STR(value);
		int err = ERR_NOT_SUPPORTED;
       // int err = _media_base_client->IO_SetRoomAttr( name, value, [=] ( int ec )
      //  {
      //      int err = TransformErrorCode( ec );
      //      auto handle = [=] { 
      //          Log.d( "Call IAudioRoomEventHandler::RespondSetRoomAttr(%s,%s,%d)\n", strname.c_str(), strvalue.c_str(), err );
      //          _handler->RespondSetRoomAttr( strname.c_str(), strvalue.c_str(), err ); };
      //      RecvAsyncEvent( std::move( handle ) );
      //  } );
        return TransformErrorCode( err );
    }
    int AudioRoomImpl::GetRoomAttr( const char* name, StringPtr& extend )
    {
        Log.v( "IAudioRoom::GetRoomAttr(%s,extend)\n", STR(name) );
        //auto str = _media_base_client->IO_GetRoomAttr( name );
		std::string str;
        extend.reset( new StringImpl( str ) );
        return ERR_OK;
    }
    int AudioRoomImpl::Login( const char* roomkey, UID uid, const char* user_extend, bool loop_try_login )
    {
        Log.v( "IAudioRoom::Login(%s,%s,%s)\n", STR( roomkey ), STR( uid ), STR( user_extend ) );
        if ( !roomkey )
        {
            Log.e( "IAudioRoom::Login 参数错误：roomkey不允许为空\n" );
            return ERR_INVALID_ROOM_KEY;
        }
        if ( !uid )
        {
            Log.e( "IAudioRoom::Login 参数错误：uid不允许为空\n" );
            return ERR_INVALID_USER_ID;
        }
        if ( !_handler )
        {
            Log.e( "IAudioRoom::Login 未注册事件回调接口\n" );
            return ERR_NOT_INITIALIZE;
        }
        if ( roomkey != _roomkey || uid != _uid )
        {
            Logout();
        }
        _roomkey = roomkey;
        _uid = uid;
        _user_extend = STR( user_extend );
        _loop_try_login = loop_try_login;
        if ( _try_logout )
        {
            _try_login = true;
            return ERR_OK;
        }
        if ( !_use_poll )
        {
            StartThreadLoop();
        }

//        int st = _media_base_client->IO_GetLoginedStatus();
        //if ( st == LSLogined )
        //{
        //    Log.d( "has login status is :%d",st );
        //    return ERR_OK;
        //}

        return DoLogin( );
    }

    int AudioRoomImpl::Logout()
    {
        Log.v( "IAudioRoom::Logout()\n" );
        if (_try_logout)
        {
            Log.w( "IAudioRoom::Logout 重复调用无效" );
            return ERR_ALREADY_IN_USE;
        }
        _try_logout = true;
        _try_login = false;
        std::string uid = _uid;
        std::string roomkey = _roomkey;
		int ec = ERR_NOT_SUPPORTED;
/*	    ec = _media_base_client->IO_Logout( [=] ( int ec )
        {
            _try_logout = false;
            int err = TransformErrorCode( ec );
            auto handle = [=] ()
            {
                Log.d( "Call IAudioRoomEventHandler::RespondLogout(%s,%s,%d)\n", _roomkey.c_str(), _uid.c_str(), err );
                _handler->RespondLogout( roomkey.c_str(), uid.c_str(), err );
                if ( _try_login )
                {
                    _try_login = false;
                    Log.d("在登出房间之后又立即登陆房间，自动重新进入\n");
                    DoLogin( );
                }
            };
            RecvAsyncEvent( std::move( handle ) );
        } );
*/
        if ( ec != 0 )
        {
            _try_logout = false;
        }
        return TransformErrorCode( ec );
    }

    int AudioRoomImpl::GetLoginStatus()
    {
        Log.v( "IAudioRoom::GetLoginStatus()\n" );
        //return _media_base_client->IO_GetLoginedStatus();
		return ERR_NOT_SUPPORTED;
    }

    void AudioRoomImpl::Poll()
    {
        Log.v( "IAudioRoom::Poll()\n" );
        if ( _use_poll )
        {
            ConsumeAllEvent();
        }
        else
        {
            Log.w("IAudioRoom::Poll 错误的调用。如果要使用轮询模式，需要在调用IAudioRoom::RegisterEventHandler时设置参数use_poll为true\n");
        }
    }

    void AudioRoomImpl::EnableRealAudio( bool enable )
    {
        if ( enable )
        {
            Log.i( "EnableRealAudio 使用实时语音功能 \n" );
        }
        else
        {
            Log.i( "EnableRealAudio 禁用实时语音功能\n" );
        }
        //_media_base_client->IO_EnableConnectAudio( enable );
    }

    void AudioRoomImpl::EnableAudioMessage( bool enable )
    {
        if ( enable )
        {
            Log.i( "EnableAudioMessage 使用消息功能 \n" );
        }
        else
        {
            Log.i( "EnableAudioMessage 禁用消息功能\n" );
        }
        //_media_base_client->IO_EnableConnectRichMessage( enable );
    }

    void AudioRoomImpl::EnablePullUserList( bool enable /*= false */ )
    {
        if ( enable )
        {
            Log.i( "EnablePullUserList 使用用户列表功能 \n" );
        }
        else
        {
            Log.i( "EnablePullUserList 禁用用户列表功能\n" );
        }
        _enable_pull_user_list = enable;
    }

    void AudioRoomImpl::RecvAsyncEvent( EventHandle handle )
    {
        if ( !_run_flag )
        {
            Log.w("房间已经退出，还收到事件通知，无法处理。\n");
            return;
        }
        std::lock_guard<std::mutex> lock( _event_handle_mutex );
        _event_handle_list.push_back( std::move( handle ) );
    }

    void AudioRoomImpl::ConsumeAllEvent()
    {
        while ( true )
        {
            EventHandle cb;
            {
                std::lock_guard<std::mutex> lock( _event_handle_mutex );

                if ( !_event_handle_list.empty() )
                {
                    cb = _event_handle_list.front();
                    _event_handle_list.pop_front();
                }
            }
            if ( cb )
            {
                cb();
            }
            else
            {
                break;
            }
        }
    }

    void AudioRoomImpl::StartThreadLoop()
    {
        if ( _async_event_process_thread.joinable() )
        {
            return;
        }
        _async_event_process_thread = std::thread( [this] ()
        {
            while ( _run_flag )
            {
                ConsumeAllEvent();
                if ( _run_flag )
                {
                    std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
                }
            }
        } );
    }

    void AudioRoomImpl::StopThreadLoop()
    {
        {
            std::lock_guard<std::mutex> lock( _event_handle_mutex );
            _event_handle_list.clear();
        }
        if ( _async_event_process_thread.joinable() )
        {
            _async_event_process_thread.join();
        }
    }

    void AudioRoomImpl::EventNotifyHandler( int eventid, const void* param )
    {
        Log.d("收到事件通知(event id:%d,param:%p)\n",eventid,param);
		/*
        using namespace snail::client::media;
        EventHandle handle;
        if (_handler)
        {
            switch ( eventid )
            {
            case event_net_broken:
            {
                handle = [this] ()
                {
                    Log.d( "Call IAudioRoomEventHandler::NotifyConnectionLost()\n" );
                    _handler->NotifyConnectionLost();
                };
            }break;
            case event_net_work:
            {
                handle = [this] ()
                {
                    Log.d( "Call IAudioRoomEventHandler::NotifyReConnected(%s,%s)\n", _roomkey.c_str(), _uid.c_str() );
                    _handler->NotifyReConnected( _roomkey.c_str(), _uid.c_str() );
                };
            }break;
            case event_asyn_logined:
            {
                handle = [this] ()
                {
                    Log.d( "Call IAudioRoomEventHandler::NotifyDuplicateLogined()\n" );
                    _handler->NotifyDuplicateLogined();
                };
            }break;

            case event_room_closed:
            {
                handle = [=] ()
                {
                    Log.d( "Call IAudioRoomEventHandler::NotifyRoomClose(%s,%d)\n", _uid.c_str(), (int)(uintptr_t)param );
                    _handler->NotifyRoomClose( _uid.c_str(), (int)(uintptr_t)param );
                };
            }break;
            case event_room_attr_changed:
            {
                key_value* attr = (key_value*)param;
                std::string name = std::move( attr->name() );
                std::string value = std::move( attr->value() );
                handle = [=] ()
                {
                    Log.d( "Call IAudioRoomEventHandler::NotifyRoomAttrChanged(%s,%s)\n", name.c_str(), value.c_str() );
                    _handler->NotifyRoomAttrChanged( name.c_str(), value.c_str() );
                };
            }break;
            default:
                break;
            }
        }
        else
        {
            Log.e( "IAudioRoomEventHandle 指针为空，不能处理相关事件\n" );
        }
		 
		if (!handle)
		{
			for (auto & module:_modules)
			{
                IModuleHandler* handler = dynamic_cast<IModuleHandler*>( module.second.get() );
				if (handler)
				{ 
					handle = handler->EventNotifyHandler(eventid, param);
					if (handle)
						break;
				}
			}
		}
		  
        if ( handle )
        {
            RecvAsyncEvent( std::move( handle ) );
        }
        else
        {
            Log.w( "收到事件通知(%d,%p),但是没有注册相关事件回调接口\n" );
        }
		*/
    }

    int AudioRoomImpl::DoLogin()
    {
		/*
        if ( _media_base_client->IO_GetLoginedStatus() == LSLogined )
        {
            return ERR_OK;
        }
        Log.d( "AudioRoomImpl::DoLogin\n" );
        std::string uid = _uid;
        std::string roomkey = _roomkey;
        int ec = _media_base_client->IO_Login( _uid.c_str(), _roomkey.c_str(), _enable_pull_user_list, _user_extend.c_str(), _loop_try_login, [=] ( int ec )
        {
            Log.d( "IAudioRoom::Login 返回结果：%s\n",ec==0?"登陆成功":"登陆失败\n" );
            if( _try_login && _media_base_client->IO_GetLoginedStatus() != LSLogined )
            {
                _try_login = false;
                ec = DoLogin();
                if ( ec == 0 )
                {
                    Log.i("IAudioRoom::Login 重新尝试登陆\n");
                    return;
                }
            }
            int err = TransformErrorCode( ec );
            auto handle = [=] () { 
                Log.d( "Call IAudioRoomEventHandler::RespondLogin(%s,%s,%d)\n", roomkey.c_str(), uid.c_str(), err );
                _handler->RespondLogin( roomkey.c_str(), uid.c_str(), err ); };
            RecvAsyncEvent( std::move( handle ) );
        } );
        return TransformErrorCode( ec );// 需要转换下
		*/
		return ERR_NOT_SUPPORTED;
    }

//////////////////////////////////////////////////////////////
    UserModuleImpl::UserModuleImpl( AudioRoomImpl*host )
    {
        _host = host;
        //_media_base_client = _host->_media_base_client.get();
    }

    void UserModuleImpl::RegisterEventHandler( IUserEventHandler* handler )
    {
        Log.v( "IUserModule::RegisterEventHandler(%p)\n",handler );
        if (!handler)
        {
            Log.w("IUserModule::RegisterEventHandler 参数错误：handler指针不允许为空\n");
        }
        _handler = handler;
        _host->EnablePullUserList( true );
    }

    int UserModuleImpl::SetUserAttr( UID uid, const char* name, const char* value )
    {
        Log.v( "IUserModule::SetUserAttr(%s,%s,%s)\n", STR( uid ), STR( name ), STR(value) );
        if (!uid)
        {
            Log.e("IUserModule::SetUserAttr 参数错误：uid不允许为空\n");
            return ERR_INVALID_ARGUMENT;
        }
        if (!name)
        {
            Log.e( "IUserModule::SetUserAttr 参数错误：name不允许为空\n" );
            return ERR_INVALID_ARGUMENT;
        }
        std::string struid = uid;
        std::string strname = STR(name);
        std::string strvalue = STR(value);
/*        int err = _media_base_client->IO_SetUserAttr( uid, name, value, [=] (int ec)
        {
            int err = TransformErrorCode( ec );
            auto handle = [=] {
                Log.d( "Call IUserEventHandler::RespondSetUserAttr(%s,%s,%s,%d)\n", struid.c_str(), strname.c_str(), strvalue.c_str(), err );
                _handler->RespondSetUserAttr( struid.c_str(), strname.c_str(), strvalue.c_str(), err ); };
            _host->RecvAsyncEvent( std::move(handle) );
        } );
        return TransformErrorCode(err);
		*/
		return ERR_NOT_SUPPORTED;
    }

    int UserModuleImpl::GetUserCount()
    {
        Log.v( "UserModule::GetUserCount()\n" );
       // int count = _media_base_client->IO_LockUserList();
       // _media_base_client->IO_UnLockUserList();
       // return count;
		return ERR_NOT_SUPPORTED;
    }

    int UserModuleImpl::GetUserList( UserListPtr& users )
    {
        Log.v( "IUserModule::GetUserList(users)\n" );
        //TODO 检查登陆状态
		/*
        auto userlist = new UserListImpl( _media_base_client,false );
        size_t count = userlist->size();
        for ( int i = 0; i < count; i++ )
        {
            userlist->_impl[i] = std::make_shared<UserImpl<IBaseUser*>>(_media_base_client->IO_GetUser(i));
        }
        users.reset( userlist );
		*/
        return ERR_OK;
    }

    int UserModuleImpl::GetUser( UID uid, UserPtr& user )
    {
        Log.v( "IUserModule::GetUser(%s,user)\n",STR(uid) );
        if (!uid)
        {
            Log.e( "IUserModule::GetUser 参数错误：uid不允许为空\n" );
            return ERR_INVALID_ARGUMENT;
        }
/*        auto p = _media_base_client->IO_GetUserByID( uid );
        if (!p)
        {
            Log.w("IUserModule::GetUser 找不到用户(%s)！\n",uid);
            return ERR_USER_NOT_FOUND;
        }
        user.reset( new UserImpl<UserPtr_t>( p ) );
		*/
        return ERR_OK;
    }

    int UserModuleImpl::KickOff( UID uid )
    {
        Log.v( "IUserModule::KickOff(%s)\n",STR(uid) );
        if ( !uid )
        {
            Log.e( "IUserModule::GetUser 参数错误：uid不允许为空\n" );
            return ERR_INVALID_ARGUMENT;
        }
        std::string struid = uid;
        /*int ec = _media_base_client->IO_KickOff( uid, [=] ( int ec )
        {
            int err = TransformErrorCode( ec );
             auto handle = [=] () {
                 Log.d( "Call IUserEventHandler::RespondKickOff(%s,%d)\n", struid.c_str(), err );
                 _handler->RespondKickOff( struid.c_str(), err ); };
             _host->RecvAsyncEvent( std::move(handle) );
        } );
        return TransformErrorCode( ec );*/
		return ERR_NOT_SUPPORTED;
    }

    EventHandle UserModuleImpl::EventNotifyHandler( int eventid, const void* param )
    {
		/*
        using namespace snail::client::media;
        EventHandle handle;
        if ( !_handler )
        {
            Log.w("IUserEventHandler::EventNotifyHandler 未注册事件处理接口\n");
            return handle;
        }
        switch ( eventid )
        {
        case event_user_logined:
        {
            IBaseUser* user = (IBaseUser*)param;
            std::string uid = user->userid()->c_str();
            handle = [=] () {
                Log.d( "Call IUserEventHandler::NotifyUserEnterRoom(%s)\n", uid.c_str() );
                _handler->NotifyUserEnterRoom( uid.c_str() ); };
        }break;
        case event_user_leave:
        {
            IUserID*user = (IUserID*)param;
            std::string uid = user->c_str();
            handle = [=] () {
                Log.d( "Call IUserEventHandler::NotifyUserLeaveRoom(%s)\n", uid.c_str() );
                _handler->NotifyUserLeaveRoom( uid.c_str() ); };
        }break;
        case event_kickoff:
        {
            IUserID*user = (IUserID*)param;
            std::string uid = user->c_str();
            handle = [=] () { 
                Log.d( "Call IUserEventHandler::NotifyKiceOff(%s)\n", uid.c_str() );
                _handler->NotifyKiceOff( uid.c_str() ); };
        }break;
        case event_user_attr_changed:
        {
            user_key_value*user_attr = (user_key_value*)param;
            std::string name = user_attr->name();
            std::string value = user_attr->value();
            std::string uid = user_attr->userid()->c_str();
            handle = [=] {
                Log.d( "Call IUserEventHandler::NotifyUserAttrChanged(%s,%s,%s)\n", uid.c_str(), name.c_str(), value.c_str() );
                _handler->NotifyUserAttrChanged( uid.c_str(), name.c_str(), value.c_str() ); };
        }break;
        case event_user_speaking:
        {
            user_speaking* us = (user_speaking*)param;
            std::string uid = us->userid()->c_str();
            int volume = us->volume();
            handle = [=] { 
                Log.d( "Call IUserEventHandler::NotifyUserSpeaking(%s,%d)\n", uid.c_str(), volume );
                _handler->NotifyUserSpeaking( uid.c_str(), volume ); };
        }break;
        default:
            break;
        }
        return handle;
		*/
		return nullptr;
    }


/////////////////////////////////////////////////////////////////
    MessageModuleImpl::MessageModuleImpl( AudioRoomImpl* host )
    {
        _host = host;
        //_media_base_client = host->_media_base_client.get();
    }

    MessageModuleImpl::~MessageModuleImpl()
    {

    }
    void MessageModuleImpl::RegisterEventHandler( IMessageEventHandler * handler )
    {
        Log.v( "IMessageModule::RegisterEventHandler(%p)\n",handler );
        if (!handler)
        {
            Log.e( "IMessageModule::RegisterEventHandler 参数错误：IMessageEventHandler指针不允许为空\n" );
        }
        _handler = handler;
        if (_handler)
        {
			_host->EnableAudioMessage(true);
        }
    }

    int MessageModuleImpl::StartRecord( bool stt_sync /*= false */ )
    {
        Log.v("IMessageModule::StartRecord(%d)\n",stt_sync);
        if ( !_handler )
        {
            Log.e( "IMessageModule::StartRecord 需要调用RegisterEventHandler函数设置回调指针\n" );
            return ERR_NOT_INITIALIZE;
        }
		/*
        int err = _media_base_client->IO_AudioMsgStartRecord( stt_sync, [this] ( int ec, const void* param )
        {
            EventHandle handle;
            const char* url = (const char*)param;
            std::string strurl = STR(url);
            int err = TransformErrorCode( ec ); 
            if ( err == 0  && strcmp(url, "#start_record") == 0)
            {
				handle = [=] { 
                    Log.d( "Call IMessageEventHandler::NotifyAudioMsgRecordBegin()\n" );
                    _handler->NotifyAudioMsgRecordBegin(); };
            }
            else
            {
				handle = [=] { 
                    Log.d( "Call IMessageEventHandler::NotifyAudioMsgRecordEnd(%s,%d)\n", strurl.c_str(), err );
                    _handler->NotifyAudioMsgRecordEnd(strurl.c_str(), err); };
            }
			
            _host->RecvAsyncEvent( std::move(handle) );
        } );
        return TransformErrorCode( err );
		*/
		return ERR_NOT_SUPPORTED;
    }

    void MessageModuleImpl::StopRecord( bool cancel /*= false */ )
    {
        Log.v( "IMessageModule::StopRecord(%d)\n", cancel );
        if (cancel)
        {
            Log.i("IMessageModule::StopRecord 取消本次录音\n");
        }
        //_media_base_client->IO_AudioMsgStopRecord(cancel);
    }

    bool MessageModuleImpl::IsRecordingAudioMsg()
    {
        Log.v( "IMessageModule::IsRecordingAudioMsg()\n" );
        //return _media_base_client->IO_IsRecordingAudioMsg();
		return false;
    }

    int MessageModuleImpl::StartPlayout( const char* url )
    {
        Log.v( "IMessageModule::StartPlayout(%s)\n",STR(url) );
        if ( !_handler )
        {
            Log.e( "IMessageModule::StartPlayout 需要调用RegisterEventHandler函数设置回调指针\n" );
            return ERR_NOT_INITIALIZE;
        }
        if ( !url)
        {
            Log.e( "IMessageModule::StartPlayout 参数错误：url不允许为空\n" );
            return ERR_INVALID_ARGUMENT;
        }
        std::string strurl = url;
        /*int ec = _media_base_client->IO_AudioMsgStartPlay( url, [=] ( int ec,const void* param )
        {
            EventHandle handle;
            const char* strparam = (const char*)param;
            if ( ec == 0 && strcmp(strparam,"#start_play") == 0)
            {
				handle = [=] { 
                    Log.d( "Call IMessageEventHandler::NotifyAudioMsgPlayBegin(%s)\n", strurl.c_str() );
                    _handler->NotifyAudioMsgPlayBegin(strurl.c_str()); };
            }
            else
            {
                ec = TransformErrorCode( ec );
				handle = [=] { 
                    Log.d( "Call IMessageEventHandler::NotifyAudioMsgPlayEnd(%s,%d)\n", strurl.c_str(), ec );
                    _handler->NotifyAudioMsgPlayEnd(strurl.c_str(), ec); };
            }
            _host->RecvAsyncEvent( std::move(handle) );
        });
        return TransformErrorCode( ec );*/
		return ERR_NOT_SUPPORTED;
    }

    void MessageModuleImpl::StopPlayout()
    {
        Log.v( "IMessageModule::StopPlayout()\n" );
        //_media_base_client->IO_AudioMsgStopPlay();
    }

    bool MessageModuleImpl::IsPlayingAudioMsg()
    {
        Log.v( "IMessageModule::IsPlayingAudioMsg()\n" );
        //return _media_base_client->IO_IsEnablePlayout();
		return false;
    }

    int MessageModuleImpl::GetAudioMsgTimeSpan( const char* url )
    {
        Log.v( "IMessageModule::GetAudioMsgTimeSpan(%s)\n",STR(url) );
        //return _media_base_client->IO_GetMsgTimeSpan(url);
		return ERR_NOT_SUPPORTED;
    }

    bool MessageModuleImpl::IsAudioMsgLocalExsit( const char* url )
    {
        Log.v( "IMessageModule::IsAudioMsgLocalExsit(%s)\n", STR( url ) );
        //return _media_base_client->IO_LocalFileIsExist(url);
		return false;
    }

    int MessageModuleImpl::SendMsg( int msgtype, const char* data, int length, const char* extends /*= 0*/, UID to_user/* = 0 */ )
    {
        Log.v( "IMessageModule::SendMsg(%d,%s,%d,%s,%s)\n", msgtype, STR( data ), length, STR(extends),STR(to_user) );
        if ( !_handler )
        {
            Log.e( "IMessageModule::SendMsg 需要调用RegisterEventHandler函数设置回调指针\n" );
            return ERR_NOT_INITIALIZE;
        }
        if (length < 0)
        {
            Log.e("IMessageModule::SendMsg 参数错误：length不能小于0\n");
            return ERR_INVALID_ARGUMENT;
        }
        std::string strdata;
        if (data)
        {
            strdata.insert(strdata.end(), data, data + length);
        }
        
        std::string strextends  = STR(extends);
        std::string str_to_user = STR(to_user);
        /*int ec = _media_base_client->IO_SendMessage( static_cast<snail::client::media::eMsgType>( msgtype ), data, length, extends, to_user, [=]( int ec, const void* param )
        {
            int msg_id = -1;
            if (ec == 0)
            {
                msg_id = (int)(uintptr_t)param;
            }
            ec = TransformErrorCode( ec );
            auto handle = [=] { 
                Log.d( "Call IMessageEventHandler::RespondSendMsg(%d,%s,%d,%s,%s,%d,%d)\n", msgtype, strdata.c_str(), strdata.length(), strextends.c_str(), str_to_user.c_str(), msg_id, ec );
                _handler->RespondSendMsg( msgtype, strdata.c_str(), strdata.length(), strextends.c_str(), str_to_user.c_str(),msg_id, ec ); };
            _host->RecvAsyncEvent( std::move(handle) );
        } );

        return TransformErrorCode( ec );*/
		return ERR_NOT_SUPPORTED;
    }

    int MessageModuleImpl::GetHistoryMsgList( int msgid, int msg_count )
    {
        Log.v( "IMessageModule::GetHistoryMsgList(%d,%d)\n", msgid,msg_count );
        if (!_handler)
        {
            Log.e("IMessageModule::GetHistoryMsgList 需要调用RegisterEventHandler函数设置回调指针\n");
            return ERR_NOT_INITIALIZE;
        }
        /*int err = _media_base_client->IO_GetMessages( msgid, msg_count, [=] ( int ec, const void* param )
        {
            EventHandle handle;
            ec = TransformErrorCode( ec );
            if ( ec == 0 )
            {
                typedef std::vector<std::shared_ptr<IBaseMessage>> BaseMessageList;
                auto msglist = (BaseMessageList*)param;
                std::shared_ptr<MessageListImpl> msgvec = std::make_shared<MessageListImpl>();
                for ( auto item : *msglist )
                {
                    msgvec->_impl.push_back( std::make_shared<MessageImpl>( item ) );
                }
				handle = [=] { 
                    Log.d( "Call IMessageEventHandler::RespondGetHitoryMsgList(%d,%p)\n", ec, msgvec.get() );
                    _handler->RespondGetHitoryMsgList(ec, msgvec.get()); };
            }
            else
            {
				handle = [=] { 
                    Log.d( "Call IMessageEventHandler::RespondGetHitoryMsgList(%d,nullptr)\n", ec );
                    _handler->RespondGetHitoryMsgList(ec, nullptr); };
            }
            _host->RecvAsyncEvent( std::move( handle ) );
        } );
        return TransformErrorCode( err );*/
		return ERR_NOT_SUPPORTED;

    }

    int MessageModuleImpl::StartSpeechToText( const char* url )
    {
        Log.v( "IMessageModule::StartSpeechToText(%s)\n", STR(url) );
        if ( !_handler )
        {
            Log.e( "IMessageModule::StartSpeechToText 需要调用RegisterEventHandler函数设置回调指针\n" );
            return ERR_NOT_INITIALIZE;
        }
        if (!url)
        {
            Log.e("IMessageModule::StartSpeechToText 参数错误：url不允许为空\n");
            return ERR_INVALID_ARGUMENT;
        }
        std::string strurl = url;
        /*int err = _media_base_client->IO_StartSTT( url, [=] ( int ec, const void* param )
        {
            std::string strtext = (const char*)param;
            auto handle = [=] {
                Log.d( "Call IMessageEventHandler::RespondSpeechToText(%s,%s,%d)\n", strurl.c_str(), strtext.c_str(), ec );
                _handler->RespondSpeechToText( strurl.c_str(), strtext.c_str(), ec ); };
            _host->RecvAsyncEvent( std::move( handle ) );
        } );
        return TransformErrorCode(err);*/
		return ERR_NOT_SUPPORTED;

    }

    void MessageModuleImpl::StopSpeechToText( bool cancel )
    {
        Log.v( "IMessageModule::StopSpeechToText(%d)\n", cancel );
        //_media_base_client->IO_CancelSTT(cancel);
    }

    bool MessageModuleImpl::IsSpeechToTextNow()
    {
        Log.v( "IMessageModule::IsSpeechToTextNow()\n" );
       // return _media_base_client->IO_IsSTT();
		return false;
    }

    int MessageModuleImpl::GetTextOfSpeech( const char* url, const char** text )
    {
        Log.v( "IMessageModule::GetTextOfSpeech(%s,%p)\n",STR(url), text );
        if ( !url )
        {
            Log.e( "IMessageModule::GetTextOfSpeech 参数错误：url不允许为空\n" );
            return ERR_INVALID_ARGUMENT;
        }
        if (!text)
        {
            Log.e("IMessageModule::GetTextOfSpeech 参数错误：text指针不允许为空\n");
            return ERR_INVALID_ARGUMENT;
        }
        
       //*text = _media_base_client->IO_GetSpeechText( url );
        return ERR_OK;
    }

    EventHandle MessageModuleImpl::EventNotifyHandler( int eventid, const void* param )
    {
        EventHandle handle;
        if ( !_handler )
        {
            Log.w( "IMessageEventHandler::EventNotifyHandler 未注册事件处理接口\n" );
            return handle;
        }
       /* switch ( eventid )
        {
        case evt_msg_recv_msglist:
        {
            auto msglist = ( std::vector< std::shared_ptr<snail::client::media::IBaseMessage> >* )param;
            std::shared_ptr<MessageListImpl> msgvec = std::make_shared<MessageListImpl>();
            for ( auto item : *msglist )
            {
                msgvec->_impl.push_back( std::make_shared<MessageImpl>( item ) );
            }
            handle = [=]
            {
                Log.d( "Call IMessageEventHandler::NotifyRecvHistoryMsgList(%p)\n", msgvec.get() );
                _handler->NotifyRecvHistoryMsgList( msgvec.get() );
            };
        }
		break;
        case evt_msg_recv_message:
        {
            auto message = (std::shared_ptr<IBaseMessage>*)param;
            std::shared_ptr<MessageImpl> msg = std::make_shared<MessageImpl>( *message );// 不要用new的形式。
            handle = [=]
            {
                Log.d( "Call IMessageEventHandler::NotifyRecvMsg(%p)\n", msg.get() );
                _handler->NotifyRecvMsg( msg.get() );
            };
        }break;
        default:
            break;
        }*/
        return handle;
    }

/////////////////////////////////////////////////////////////////////
    RealAudioModuleImpl::RealAudioModuleImpl( AudioRoomImpl* host )
    {
        _host = host;
        //_media_base_client = _host->_media_base_client.get();
    }

    RealAudioModuleImpl::~RealAudioModuleImpl()
    {

    }

    void RealAudioModuleImpl::RegisterEventHandler( IRealAudioEventHandler* handler )
    {
        Log.v( "IRealAudioModule::RegisterEventHandler(%p)\n", handler );
        if ( !handler)
        {
            Log.e("IRealAudioModule::RegisterEventHandler handler指针不允许为空\n");
        }
        _handler = handler;
        if (_handler)
        {
            _host->EnableRealAudio( true );
        }

    }

    int RealAudioModuleImpl::BlockUser( UID uid, bool block )
    {
        Log.v( "IRealAudioModule::RegisterEventHandler(%s,%d)\n", STR(uid), block );
        std::string struid = uid;
        /*int ec = _media_base_client->IO_BlockUser( uid, block, [=] ( int ec )
        {
            int err = TransformErrorCode( ec );
            auto handle = [=] {
                Log.d( "Call IRealAudioEventHandler::RespondBlockUser(%s,%d,%d)\n", struid.c_str(), block, err );
                _handler->RespondBlockUser( struid.c_str(), block, err ); };
            _host->RecvAsyncEvent( std::move( handle ) );
        } );
        return TransformErrorCode(ec);*/
		return ERR_NOT_SUPPORTED;
    }

    int RealAudioModuleImpl::DisableSpeaking( UID uid, bool disspeak )
    {
        Log.v( "IRealAudioModule::DisableSpeaking(%s,%d)\n", STR( uid ), disspeak );
        std::string struid = uid;
       /* int ec = _media_base_client->IO_DisableSpeaking( uid, disspeak, [=] ( int ec )
        {
            int err = TransformErrorCode( ec );
            auto handle = [=] {
                Log.d( "Call IRealAudioEventHandler::RespondDisableSpeaking(%s,%d,%d)\n", struid.c_str(), disspeak, err );
                _handler->RespondDisableSpeaking( struid.c_str(), disspeak, err ); };
            _host->RecvAsyncEvent( std::move( handle ) );
        } );
        return TransformErrorCode( ec );*/
		return ERR_NOT_SUPPORTED;
    }

    void RealAudioModuleImpl::EnableSpeak( bool enable )
    {
        Log.v( "IRealAudioModule::EnableSpeak(%d)\n", enable );
       // _media_base_client->IO_EnableSpeak( enable );
    }

    bool RealAudioModuleImpl::IsEnableSpeak()
    {
        Log.v( "IRealAudioModule::IsEnableSpeak()\n" );
      //  return _media_base_client->IO_IsEnableSpeak();
		return false;
    }

    void RealAudioModuleImpl::EnablePlayout( bool enable )
    {
        Log.v( "IRealAudioModule::EnablePlayout(%d)\n", enable );
        //_media_base_client->IO_EnablePlayout( enable );
    }

    bool RealAudioModuleImpl::IsEnablePlayout()
    {
        Log.v( "IRealAudioModule::IsEnablePlayout()\n" );
       // return _media_base_client->IO_IsEnablePlayout();
		return false;
    }

    EventHandle RealAudioModuleImpl::EventNotifyHandler( int eventid, const void* param )
    {
        EventHandle handle;
        if ( !_handler )
        {
            Log.w("IRealAudioEventHandler::EventNotifyHandler 未注册事件处理接口\n");
            return handle;
        }
        /*switch ( eventid )
        {
        case event_disable_speaking:
        {
            IBaseUser* user = (IBaseUser*)param;
            std::string uid = user->userid()->c_str();
            bool disable = user->IsDisableSpeak();
            handle = [=] { 
                Log.d( "Call IRealAudioEventHandler::NotifyDisableSpeaking(%s,%d)\n", uid.c_str(), disable );
                _handler->NotifyDisableSpeaking( uid.c_str(), disable ); };
        }break;
        default:
            break;
        }*/
        return handle;
    }

///////////////////////////////////////////////////////////////////////////
    MicOrderModuleImpl::MicOrderModuleImpl( AudioRoomImpl* host )
    {
        _host = host;
        //_media_base_client = _host->_media_base_client.get();
    }

    MicOrderModuleImpl::~MicOrderModuleImpl()
    {

    }

    void MicOrderModuleImpl::RegisterEventHandler( IMicOrderEventHandler* handler )
    {
        Log.v( "IMicOrderModule::RegisterEventHandler(%p)\n",handler );
        if ( !handler)
        {
            Log.e( "IMicOrderModule::RegisterEventHandler handler指针不允许为空\n" );
        }
        _handler = handler;

    }

    int MicOrderModuleImpl::AddToMicList( UID uid )
    {
        Log.v( "IMicOrderModule::AddToMicList(%s)\n", STR( uid ) );
        std::string struid = uid;
        //int ec = _media_base_client->IO_AddToMicList( uid, [=] ( int ec ) 
        //{
        //    ec = TransformErrorCode( ec );
        //    auto handle = [=] {
        //        Log.d( "Call IMicOrderEventHandler::RespondAddToMicList(%s,%d)\n", struid.c_str(), ec );
        //        _handler->RespondAddToMicList(struid.c_str(),ec); };
        //    _host->RecvAsyncEvent( std::move( handle ) );
        //} );
        //return TransformErrorCode(ec);
		return ERR_NOT_SUPPORTED;
    }

    int MicOrderModuleImpl::RemoveFromMicList( UID uid )
    {
        Log.v( "IMicOrderModule::RemoveFromMicList(%s)\n", STR( uid ) );
        std::string struid = uid;
        //int ec = _media_base_client->IO_RemoveFromMicList( uid, [=] ( int ec )
        //{
        //    ec = TransformErrorCode( ec );
        //    auto handle = [=] {
        //        Log.d( "Call IMicOrderEventHandler::RespondRemoveFromMicList(%s,%d)\n", struid.c_str(), ec );
        //        _handler->RespondRemoveFromMicList( struid.c_str(), ec ); };
        //    _host->RecvAsyncEvent( std::move( handle ) );
        //} );
        //return TransformErrorCode( ec );
		return ERR_NOT_SUPPORTED;
    }

    int MicOrderModuleImpl::SetMicOrder( UID uid, int order )
    {
        Log.v( "IMicOrderModule::SetMicOrder(%s,%d)\n", STR( uid ), order );
        std::string struid = uid;
        //int ec = _media_base_client->IO_SetUserMicOrder( uid, order, [=](int ec ) 
        //{
        //    ec = TransformErrorCode( ec );
        //    auto handle = [=] {
        //        Log.d( "Call IMicOrderEventHandler::RespondSetMicOrder(%s,%d)\n", struid.c_str(), order );
        //        _handler->RespondSetMicOrder( struid.c_str(), order ); };
        //    _host->RecvAsyncEvent( std::move( handle ) );
        //} );
        //
        //return TransformErrorCode(ec);
		return ERR_NOT_SUPPORTED;

    }

    int MicOrderModuleImpl::GetMicOrder( UID uid )
    {
        Log.v( "IMicOrderModule::GetMicOrder(%d)\n", STR( uid ) );
       // return _media_base_client->IO_GetMicOrder(uid);
		return ERR_NOT_SUPPORTED;

    }

    int MicOrderModuleImpl::GetMicUserCount()
    {
        Log.v( "IMicOrderModule::GetMicUserCount()\n" );
        //int count = _media_base_client->IO_LockMicList();
        //_media_base_client->IO_UnLockMicList();
        //return count;
		return ERR_NOT_SUPPORTED;

    }

    int MicOrderModuleImpl::GetMicUserList( UserListPtr& list )
    {
        Log.v( "IMicOrderModule::GetMicUserList(list)\n" );
        //_media_base_client->IO_LockMicList();
        //UserListImpl* userlist = new UserListImpl( _media_base_client, true );
        //int count = userlist->_impl.size();
        //for ( int i = 0; i < count; i++ )
        //{
        //    auto user = _media_base_client->IO_GetMicUser(i);
        //    userlist->_impl[i] = std::make_shared<UserImpl<IBaseUser*>>(user);
        //}
        //list.reset( userlist );
        return ERR_OK;
    }

    int MicOrderModuleImpl::GetMicUser( UID uid, UserPtr& user )
    {
        Log.v( "IMicOrderModule::GetMicUser(%s,user)\n",STR(uid) );
        if (!uid)
        {
            Log.e("IMicOrderModule::GetMicUser 参数错误：uid不允许为空\n");
            return ERR_INVALID_ARGUMENT;
        }
        //auto p = _media_base_client->IO_GetUserByID( uid );
        //if (!p)
        //{
        //    Log.w( "IMicOrderModule::GetMicUser 找不到用户(%s)！\n", uid );
        //    return ERR_USER_NOT_FOUND;
        //}
        //user.reset( new UserImpl<UserPtr_t>( p ) );
        return ERR_OK;
    }

    EventHandle MicOrderModuleImpl::EventNotifyHandler( int eventid, const void* param )
    {
        EventHandle handle;
        if ( !_handler )
        {
            Log.w( "收到事件通知(%d,%p),但是没有注册相关事件回调接口\n" );
            return handle;
        }
        /*switch ( eventid )
        {
        case event_miclist_user_join:
        {
            IUserID* userid = (IUserID*)param;
            std::string struid = userid->c_str();
            handle = [=] { 
                Log.d( "Call IMicOrderEventHandler::NotifyMicOrderUserJoin(%s)\n", struid.c_str() );
                _handler->NotifyMicOrderUserJoin( struid.c_str() );
            };
        }break;
        case event_miclist_user_leave:
        {
            IUserID* userid = (IUserID*)param;
            std::string struid = userid->c_str();
            handle = [=] {
                Log.d( "Call IMicOrderEventHandler::NotifyMicOrderUserLeave(%s)\n", struid.c_str() );
                _handler->NotifyMicOrderUserLeave( struid.c_str() );
            };
        }break;
        case event_miclist_user_changed:
        {
            IUserID* userid = (IUserID*)param;
            std::string struid = userid->c_str();
            
            handle = [=] { 
                Log.d( "Call IMicOrderEventHandler::NotifyMicOrderUserChanged(%s)\n", struid.c_str() );
                _handler->NotifyMicOrderUserChanged( struid.c_str() ); };
        }break;
        case event_miclist_changed:
        {
            handle = [=] {
                Log.d( "Call IMicOrderEventHandler::NotifyMicOrderChanged()\n" );
                _handler->NotifyMicOrderChanged(); };
        }break;
        case event_miclist_clear:
        {
            handle = [=] { 
                Log.d( "Call IMicOrderEventHandler::NotifyMicOrderClear()\n" );
                _handler->NotifyMicOrderClear(); };
        }break;
        }*/
        return handle;
    }


////////////////////////////////////////////////////////////////////
    VolumeModuleImpl::VolumeModuleImpl( AudioRoomImpl* host )
    {
        _host = host;
        _handler = _host->_handler;
       // _media_base_client = _host->_media_base_client.get();
    }
    VolumeModuleImpl::~VolumeModuleImpl()
    {

    }
    int VolumeModuleImpl::AdjustAudioVolume( UID uid, int volume )
    {
        Log.v( "IVolumeModule::AdjustAudioVolume(%s,%d)\n",STR(uid),volume );
       // _media_base_client->IO_GetModule( module_audio );
        return ERR_NOT_SUPPORTED;
    }

    int VolumeModuleImpl::AdjustPlaybackVolume( int volume )
    {
        Log.v( "IVolumeModule::AdjustPlaybackVolume(%d)\n", volume );
        return ERR_NOT_SUPPORTED;
    }

    int VolumeModuleImpl::AdjustRecordingVolume( int volume )
    {
        Log.v( "IVolumeModule::AdjustRecordingVolume(%d)\n", volume );
        return ERR_NOT_SUPPORTED;
    }

    int VolumeModuleImpl::GetPlaybackDeviceVolume()
    {
        Log.v( "IVolumeModule::GetPlaybackDeviceVolume()\n" );
        return ERR_NOT_SUPPORTED;
    }

}}
