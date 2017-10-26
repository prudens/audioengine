#include "audio_parse_param.h"
//#include "macrodef.h"
#include "stdlib.h"
namespace snail {
    namespace audio {
        AudioParseParameter::AudioParseParameter()
        {
            m_bEnable = true;
        }

        AudioParseParameter::~AudioParseParameter()
        {
        }

        AudioParseParameter& AudioParseParameter::GetInstance()
        {
            static AudioParseParameter _inst;
            return _inst;
        }

//         void AudioParseParameter::Enable( bool bEnable )
//         {
//             m_bEnable = bEnable;
//         }

        void AudioParseParameter::RegisterNotify( IParseParamNotify* notify )
        {
            if ( !notify )
            {
                return;
            }
            std::lock_guard<std::mutex> ls( m_lockNotify );
            m_vecNotify.push_back( notify );
        }

        void AudioParseParameter::UnRegisterNotify( IParseParamNotify* notify )
        {
            if ( !notify )
            {
                return;
            }
            std::lock_guard<std::mutex> ls( m_lockNotify );
            for ( auto it = m_vecNotify.begin(); it != m_vecNotify.end(); ++it )
            {
                if ( *it == notify )
                {
                    m_vecNotify.erase( it );
                    break;
                }
            }
        }

        void AudioParseParameter::SetParam( std::string strParam )
        {
            // 先去除空格等无效符号
            SkipBlank( strParam );

            {
                std::lock_guard<std::mutex> ls( m_lock );
                if ( m_strParam == strParam )
                {
                    return;
                }
                m_strParam = strParam;
                Parse();
            }
    {
        std::lock_guard<std::mutex> ls( m_lockNotify );
        for ( auto notify : m_vecNotify )
        {
            notify->ParseParamNotify( m_strParam );
        }
    }

        }

        int16_t AudioParseParameter::Parse()
        {
            auto ItBegin = m_strParam.begin();
            auto ItEnd = m_strParam.end();
            while ( ItBegin != ItEnd )
            {
                auto ItP1 = ItBegin;
                while ( *++ItP1 != '[' && ItP1 != ItEnd );
                if ( *ItP1 != '[' ) return -2;
                auto ItP2 = ItP1;
                while ( *++ItP2 != ']' && ItP2 != ItEnd );
                if ( *ItP2 != ']' ) return -2;

                std::string strKey( ItBegin, ItP1 );
                std::unordered_map<int8_t, std::string> second_k_v;
                while ( ItP1 != ItP2 )
                {
                    // 解析第二级k-v
                    int8_t c = *++ItP1;
                    if ( !isalpha( c ) )
                    {
                        return -1;
                    }
                    ++ItP1;
                    if ( *ItP1 != ':' ) return -2;
                    ++ItP1;
                    ItBegin = ItP1;

                    while ( *++ItP1 != ',' && ItP1 != ItP2 ) {}
                    std::string strValue( ItBegin, ItP1 );
                    second_k_v[c] = std::move( strValue );
                }
                ++ItP2;
                ItBegin = ItP2;
                m_mapParam[strKey] = std::move( second_k_v );
            }

            return 0;
        }

        void AudioParseParameter::SkipBlank( std::string& str )
        {
            std::string strOut;
            for ( auto c : str )
            {
                if ( c != ' ' && c != '\t' && c != '\r' && c != '\n' )
                {
                    strOut.append( 1, tolower( c ) );
                }
            }
            str = std::move( strOut );
        }

        bool AudioParseParameter::GetValue( const std::string& type, int8_t ch, int32_t& value )
        {
            if ( !m_bEnable )
            {
                return false;
            }
            std::string strValue;
            if ( GetValue( type, ch, strValue ) )
            {
                value = atoi( strValue.c_str() );
                return true;
            }
            return false;
        }

        bool AudioParseParameter::GetValue( const std::string& type, int8_t ch, double& value )
        {
            if ( !m_bEnable )
            {
                return false;
            }
            std::string strValue;
            if ( GetValue( type, ch, strValue ) )
            {
                value = strtod( strValue.c_str(), nullptr );
                return true;
            }

            return false;
        }

        bool AudioParseParameter::GetValue( const std::string& type, int8_t ch, int32_t& value, int32_t validStart, int32_t validEnd )
        {
            return GetValue( type, ch, value ) && value >= validStart && value < validEnd;
        }

        bool AudioParseParameter::GetValue( const std::string& type, int8_t ch, double& value, double validStart, double validEnd )
        {
            return GetValue( type, ch, value ) && value >= validStart && value < validEnd;
        }

        bool AudioParseParameter::GetValue( const std::string& type, int8_t ch, std::string& value )
        {
            if ( !m_bEnable )
            {
                return false;
            }
            std::lock_guard<std::mutex> ls( m_lock );
            auto it = m_mapParam.find( type );
            if ( it == m_mapParam.end() )
            {
                return false;
            }
            auto it2 = it->second.find( ch );
            if ( it2 == it->second.end() )
            {
                return false;
            }
            value = it2->second;
            return !value.empty();
        }

    }
}