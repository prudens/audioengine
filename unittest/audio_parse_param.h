#pragma once
/*!
 * \file audio_parse_param.h
 * \date 2016/06/01 13:28
 *
 * \author zhangnaigan
 * Contact: zhangng@snailgame.net
 *
 * \brief 语音模块解析参数，这些参数包括但不限于audio_effect，音量等控制，所有的参数都应该通过这个来配置
 *
 * TODO: 目前参数数量比较少，解析很快，但是每次有改变，都会通知所有观察者，
         观察者会重新遍历，这可能在后期会导致消耗cpu， 所以后面还需要优化。
 *
 * \note 线程安全
*/
#include <unordered_map>
#include <string>
#include <stdint.h>
#include <vector>
#include <mutex>
namespace snail {
    namespace audio {
        class IParseParamNotify;
        class AudioParseParameter
        {
        public:
            static AudioParseParameter& GetInstance();
          //  void Enable( bool bEnable );
            void RegisterNotify( IParseParamNotify* notify );
            void UnRegisterNotify( IParseParamNotify* notify );
            void SetParam( std::string strParam );
            bool GetValue( const std::string& type, int8_t ch, int32_t& value );
            bool GetValue( const std::string& type, int8_t ch, double& value );
            bool GetValue( const std::string& type, int8_t ch, int32_t& value, int32_t validStart, int32_t validEnd );//[validStart, validEnd)
            bool GetValue( const std::string& type, int8_t ch, double& value, double validStart, double validEnd );
            bool GetValue( const std::string& type, int8_t ch, std::string& value );
        private:
            AudioParseParameter();
            ~AudioParseParameter();
            int16_t Parse();
            void SkipBlank( std::string& strParam );
            void operator=( const AudioParseParameter& ) = delete;
            AudioParseParameter( const AudioParseParameter& ) = delete;
        private:
            typedef std::unordered_map<std::string, std::unordered_map<int8_t, std::string>> ParamMap;
            ParamMap m_mapParam;
            bool m_bEnable;
            std::string m_strParam;
            std::vector<IParseParamNotify*> m_vecNotify;
            std::mutex m_lock;
            std::mutex m_lockNotify;
        };

        class IParseParamNotify
        {
        public:
            virtual ~IParseParamNotify() { AudioParseParameter::GetInstance().UnRegisterNotify( this ); }
            virtual void ParseParamNotify( const std::string& Param ) = 0;
        };

    }
}