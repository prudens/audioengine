#pragma once
#include<memory> 
#include <vector>
#include <string>
#include "SnailAudioEngineHelper.h"
#include "user_service.pb.h"
#include "../sdk/user_list.h"
namespace audio_engine{
        struct AudioSDKCfg
        {
            bool _init_android = false;
            bool _set_autho_key = false;
            bool _set_cfg_folder = false;
            std::string _cfg_path;
            std::string _cfg_locale = "zh_cn";
            int  _log_level = -1;
            int  _regionver = -1;
            bool _init = false;
        };

        class StringImpl :public String
        {
        public:
            StringImpl( std::string str )
            {
                _impl = std::move( str );
            }
            virtual ~StringImpl() {}
            virtual bool empty() const
            {
                return _impl.empty();
            }
            virtual const char* c_str()
            {
                return _impl.c_str();
            }
            virtual const char* data()
            {
                return _impl.data();
            }
            virtual size_t length()
            {
                return _impl.length();
            }
            virtual void release()
            {
                delete this;
            }
        private:
            std::string _impl;
        };
        struct UserImpl :public User
        {
            explicit UserImpl( ConstMemberPtr impl )
            {
                _impl = impl;
            }
            virtual ~UserImpl() {}
            virtual const char* userid()const override
            {
                return _impl->GetUserID().c_str();
            }
            virtual const char* extends()const override
            {
                return _impl->GetUserExtend().c_str();
            }
            virtual bool IsDisableSpeak()const override
            {
                return _impl->GetState() & USER_STATE::STATE_BANNED;
            }
            virtual bool IsBlocked()const override
            {
				return _impl->GetState() & USER_STATE::STATE_BLOCKED;
            }
            virtual void release()override
            {
                delete this;
            }
			ConstMemberPtr _impl;
        };

        struct MessageImpl : public Message
        {
            virtual int msgid()const override
            {
                //return _impl->msgid();
				return 0;
            }
            virtual const char* fromuid() const override
            {
                //return _impl->from_userid();
				return "";
            }
            virtual const char* extends()const override
            {
                //return _impl->extends();
				return nullptr;
            }
            virtual uint64_t  msgtime()const override
            {
                //return _impl->msgtime();
				return 0;
            }
            virtual int msgtype()const override
            {
                //return _impl->msgtype();
				return 0;
            }
            virtual const char* content() const override
            {
                //return _impl->content();
				return nullptr;
            }
            virtual int length() const override
            {
                int length = 0;
                //_impl->content( &length );
                return length;
            }
            //MessageImpl( std::shared_ptr<snail::client::media::IBaseMessage> impl )
            //{
            //    _impl = impl;
            //}
            virtual ~MessageImpl()
            {
            }
            //std::shared_ptr<snail::client::media::IBaseMessage> _impl;
        };

        struct UserListImpl : public UserList
        {
			virtual User* operator[](size_t idx)const override { return _impl[idx].get(); }
			virtual User* at(size_t idx)const override { return _impl.at( idx ).get(); }
			virtual size_t size()const override { return _impl.size(); }
            virtual void release()override { delete this; }
            std::vector<std::shared_ptr<User>> _impl;
        };

        class MessageListImpl : public MessageList
        {
        public:
            Message* operator[]( size_t idx )const override { return _impl[idx].get(); }
            Message* at( size_t idx )const override { return _impl.at( idx ).get(); }
            size_t size()const override { return _impl.size(); }
            void release()override { delete this; }
            ~MessageListImpl()
            {
            }
            std::vector<std::shared_ptr<MessageImpl>> _impl;
        };
        struct ErrorCodeDesc
        {
            int ec;
            const char* desc;
        };

        class SDKLog
        {
        public:
            void setLevel( int level )
            {
                if ( level == LEVEL_CLOSE )
                {
                    level = LEVEL_ERROR + 1;
                }
                _level = level;
            }
            template <typename ... Args>
            void v( const char* fmt, Args const & ... args )
            {
                if ( _level <= LEVEL_VERBOSE )
                {
//                    logcat_impl( fmt, args... );
                }
            }

            template <typename ... Args>
            void d( const char* fmt, Args const & ... args )
            {
                if ( _level <= LEVEL_DEBUG )
                {
//                    logcat_impl( fmt, args... );
                }
            }
            template <typename ... Args>
            void i( const char* fmt, Args const & ... args )
            {
                if ( _level <= LEVEL_INFO )
                {
//                    logcat_impl( fmt, args... );
                }
            }
            template <typename ... Args>
            void w( const char* fmt, Args const & ... args )
            {
                if ( _level <= LEVEL_WARN )
                {
//                    logcat_impl( fmt, args... );
                }
            }
            template <typename ... Args>
            void e( const char* fmt, Args const & ... args )
            {
                if ( _level <= LEVEL_ERROR )
                {
//                    logcat_impl( fmt, args... );
                }
            }
        private:
            int _level = LEVEL_ERROR + 1;
        };
		#define ERROR_CODE_COUNT 30
        extern ErrorCodeDesc g_error_code_zh_cn[ERROR_CODE_COUNT];
        extern ErrorCodeDesc g_error_code_en_us[ERROR_CODE_COUNT];

        int TransformErrorCode( int ec );
}