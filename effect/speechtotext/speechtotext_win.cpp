
#ifdef _WIN32
#include <thread> 
#include <chrono>
#include "speechtotext_win.h"
#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include "system/system.h"
#include "string_cvt.h"
#define HINTS_SIZE  100
const char* session_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = gb2312";

SpeechToTextWin::SpeechToTextWin( std::string appid )
{
    aud_stat_ = MSP_AUDIO_SAMPLE_CONTINUE;		//音频状态
    ep_stat_ = MSP_EP_LOOKING_FOR_SPEECH;		//端点检测
    rec_stat_ = MSP_REC_STATUS_SUCCESS;			//识别状态
    int	ret = -1;
    /* 用户登录 */
    std::string login_params = "appid="+appid+",work_dir=."; //登录参数,appid与msc库绑定,请勿随意改动

    ret = MSPLogin( NULL, NULL, login_params.c_str() ); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数
    if ( MSP_SUCCESS != ret )
    {
        logcat( "MSPLogin failed, error code: %d.\n", ret );
        return;
    }
}

SpeechToTextWin::~SpeechToTextWin()
{
    MSPLogout(); //退出登录
}

int  SpeechToTextWin::Write( const char* audioSample, std::size_t nSamples )
{
    aud_stat_ = MSP_AUDIO_SAMPLE_CONTINUE;		//音频状态
    if ( count == 0 )
    {
        count += nSamples;
        aud_stat_ = MSP_AUDIO_SAMPLE_FIRST;
    }

    int ret = QISRAudioWrite( session_id_, (const void *)audioSample, nSamples, aud_stat_, &ep_stat_, &rec_stat_ );
    if ( MSP_REC_STATUS_SUCCESS == rec_stat_ )
    {
        // 已经有部分结果
        const char* rslt = QISRGetResult( session_id_, &rec_stat_, 0, &ret );
        if ( MSP_SUCCESS != ret )
        {
            return 1;
        }
        if ( rslt )
        {
            result_.append( rslt );
        }
    }
    if ( MSP_EP_AFTER_SPEECH == ep_stat_ )
    {
        return 0;
    }
    if ( MSP_SUCCESS != ret )
    {
        return -1;
    }
    printf( "write success!\n" );
    {
        return 1;
    }
}

bool SpeechToTextWin::Start()
{
    int ec = -1;
    session_id_ = QISRSessionBegin( nullptr, session_begin_params, &ec );
    if ( ec != MSP_SUCCESS || !session_id_ )
    {
        return false;
    }
    aud_stat_ = MSP_AUDIO_SAMPLE_FIRST;		//音频状态
    return true;
}

void SpeechToTextWin::Finish()
{
    int ec = -1;
    ec = QISRAudioWrite( session_id_, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat_, &rec_stat_ );
    if ( MSP_SUCCESS != ec )
    {
        return;
    }
}

int SpeechToTextWin::GetResult( std::string& strText )
{
    int errcode = 1;
    int rec_stat = 0;
    char			hints[HINTS_SIZE] = { '\0' };               //hints为结束本次会话的原因描述，由用户自定义
    if ( !session_id_ )
    {
        return -2;
    }
    const char *rslt = QISRGetResult( session_id_, &rec_stat, 0, &errcode );
    if ( MSP_SUCCESS != errcode )
    {
        QISRSessionEnd( session_id_, hints );
        session_id_ = nullptr;
        count = 0;
        return -1;
    }
    if ( NULL != rslt )
    {
        result_.append( rslt );
    }
    if ( MSP_REC_STATUS_COMPLETE == rec_stat )
    {
        QISRSessionEnd( session_id_, hints );
        session_id_ = nullptr;
        count = 0;
        strText = std::move( result_ );
        strText = mbstoutf8( strText ); //全部转为utf8格式
        return 1;
    }
    else
    {
        strText = result_; // 先返回部分结果
        return 0;
    }
}

void SpeechToTextWin::Cancel()
{
    if (session_id_)
    {
        QISRSessionEnd( session_id_, "user cancel" );
    }
}


#endif//_WIN32