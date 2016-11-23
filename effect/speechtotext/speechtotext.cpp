#include "speechtotext.h"
#include "audio_parse_param.h"
#ifdef _WIN32
#include "speechtotext_win.h"
#endif
#ifdef _ANDROID
#include "speechtotext_android.h"
#endif
#ifdef _IOS
#include "speechtotext_ios.h"
#endif
SpeechToText* SpeechToText::Create()
{
    std::string strAppid;
#ifdef _WIN32
    if ( !AudioParseParameter::GetInstance().GetValue( "sttappid", 'w', strAppid ) )
    {
        strAppid = "58044e47";
    }
    return new SpeechToTextWin( strAppid );
#elif _ANDROID

    if ( !AudioParseParameter::GetInstance().GetValue( "sttappid", 'a', strAppid ) )
    {
        strAppid = "5808722a";
    }
    auto p =  new SpeechToTextAndroid( );
    p->Init( strAppid );
    return p;
#elif _IOS
    if ( !AudioParseParameter::GetInstance().GetValue( "sttappid", 'i', strAppid ) )
    {
        strAppid = "580872f8";
    }
    return new SpeechToTextIOS(strAppid);
#else
#error "请指定一个平台"
#endif
}
