#pragma once
/*!
 * \file android_audiototext.h
 *
 * \author zhangnaigan
 * \date 十月 2016
 *
 * 目前只能在主线程运行
 */

#ifdef _ANDROID

#include <string>
#include <memory>
#include <list>
#include <mutex>
#include <cstdint>
#include "jvm_manager.h"
#include "SpeechToText.h"
class SpeechToTextAndroid:public SpeechToText
{
public:
    enum STTStatus
    {
        eSSTOK = 0,
        eInitModuleFailed = 10,
        eCreateInstFailed = 11,
        eDisableCloudEngine = 12,
        eDisableAudioSource = 13,
        eDisableResultType  = 14,
        eWriteDataFailed    = 15, 
        eSTTBeginofSpeech   = 16,
        eSTTEndofSpeech     = 17,
        eInternelError =50,

    };
    SpeechToTextAndroid();
    ~SpeechToTextAndroid();
    void Init(std::string appid);
    bool Start();
    void Cancel();
    void Finish();
    bool isStop();
    int  Write( const char* audioSample, std::size_t nSamples );
    void CacheBuffer(jobject buffer);
    int  NeedMoreSpeechData();
    void CacheResult(jstring result);
    void UpdateStatus(int status);
    int  GetResult( std::string &strText );
private:
    void Clear();
    std::mutex   write_lock_;
    std::pair<char*, int> buffer_cache_;
    std::string result_;
    bool bStop_ = true;
    int stt_status_ = eSSTOK;
    bool has_result_ = false;
};

#endif // defined(_ANDROID) 