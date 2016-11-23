#ifdef _ANDROID

#include "speechtotext_android.h"
#include <system/system.h>
#include "macrodef.h"
#include "android_device/com_snail_audio_AudioDevice.h"

SpeechToTextAndroid::SpeechToTextAndroid()
{
}

SpeechToTextAndroid::~SpeechToTextAndroid()
{
    Clear();
    Cancel();
}

void SpeechToTextAndroid::Init( std::string appid )
{
    JVMManager::GetInstance()->CallVoidMethod( JVMManager::JAVA_OBJ_AudioToText, "Init", "(J)V", (jlong)this );
}

bool SpeechToTextAndroid::Start()
{
    int ret = JVMManager::GetInstance()->CallIntMethod( JVMManager::JAVA_OBJ_AudioToText, "start", "()I" );
    bStop_ = ret != 0;
    return !bStop_;
}

void SpeechToTextAndroid::Cancel()
{
    JVMManager::GetInstance()->CallVoidMethod( JVMManager::JAVA_OBJ_AudioToText, "cancel", "()V" );
    bStop_ = true;
}

void SpeechToTextAndroid::Finish()
{
    JVMManager::GetInstance()->CallVoidMethod( JVMManager::JAVA_OBJ_AudioToText, "stop", "()V" );

    bStop_ = true;
}

bool SpeechToTextAndroid::isStop()
{
    return bStop_;
}

int SpeechToTextAndroid::Write( const char* audioSample, std::size_t nSample )
{
    if (bStop_)
    {
        return 0;
    }

    int len = std::min( buffer_cache_.second, (int)nSample );
    memcpy( buffer_cache_.first, audioSample, len );
    JVMManager::GetInstance()->CallIntMethod( JVMManager::JAVA_OBJ_AudioToText, "writeAudio", "(I)I", len );
    return true;
}

void SpeechToTextAndroid::CacheBuffer( jobject buffer )
{
    buffer_cache_.first =nullptr;
    buffer_cache_.second = 0;
    auto env = JVMManager::GetInstance()->Env();
    if ( !env )
    {
        return;
    }
    
    void* buffer_addr = env->GetDirectBufferAddress( buffer );
    jlong rec_capacity = env->GetDirectBufferCapacity( buffer );
    buffer_cache_.first = (char*)buffer_addr;
    buffer_cache_.second = static_cast<int>(rec_capacity);
}

int SpeechToTextAndroid::NeedMoreSpeechData()
{
    return 0;
}

void SpeechToTextAndroid::CacheResult( jstring result )
{
    has_result_ = true;
    auto env = JVMManager::GetInstance()->Env();
    if ( !env )
    {
        return;
    }
    const char* strResult = env->GetStringUTFChars( result, nullptr );
    if (strResult!= nullptr)
    {
        result_.assign( (const char*)strResult );
    }

    // 回调给上层
    env->ReleaseStringUTFChars( result, strResult );

    logcat( "Recognize Result:%s", result_.c_str() );

}

void SpeechToTextAndroid::UpdateStatus( int status )
{
    logcat( "[SpeechToTextAndroid::UpdateStatus]status=%d",status );
    stt_status_ = status;
    if (stt_status_ == eSTTEndofSpeech || stt_status_ < eWriteDataFailed || stt_status_ > eInternelError )
    {
        bStop_ = true;
    }
}

int SpeechToTextAndroid::GetResult( std::string &strText )
{
    if ( has_result_ )
    {
        strText = result_;
        return 1;
    }
    else if ( stt_status_ != eSSTOK && (stt_status_ < eWriteDataFailed || stt_status_ > eInternelError) )
    {
        return -1;
    }
    else
    {
        return 0;
    }
    return 0;
}

void SpeechToTextAndroid::Clear()
{
    result_.clear();
    has_result_ = false;
}

/*
* Class:     com_snail_audio_AudioToText
* Method:    CacheBufferAddress
* Signature: (JLjava/nio/ByteBuffer;)V
*/
JNIEXPORT void JNICALL Java_com_snail_audio_AudioToText_CacheBufferAddress
( JNIEnv *, jobject, jlong stt_inst, jobject buffer )
{

    SpeechToTextAndroid*pThis = (SpeechToTextAndroid*)stt_inst;
    pThis->CacheBuffer( buffer );
}

/*
* Class:     com_snail_audio_AudioToText
* Method:    NeedMoreSpeechData
* Signature: ()V
*/
JNIEXPORT jint JNICALL Java_com_snail_audio_AudioToText_NeedMoreSpeechData
( JNIEnv *, jobject, jlong stt_inst )
{
    SpeechToTextAndroid*pThis = (SpeechToTextAndroid*)stt_inst;
    return pThis->NeedMoreSpeechData();
}

/*
* Class:     com_snail_audio_AudioToText
* Method:    CacheResult
* Signature: (JLjava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_com_snail_audio_AudioToText_CacheResult
( JNIEnv *, jobject, jlong stt_inst, jstring result )
{
    SpeechToTextAndroid*pThis = (SpeechToTextAndroid*)stt_inst;
    pThis->CacheResult( result );
}

/*
* Class:     com_snail_audio_AudioToText
* Method:    UpdateStatus
* Signature: (JI)V
*/
JNIEXPORT void JNICALL Java_com_snail_audio_AudioToText_UpdateStatus
( JNIEnv *, jobject, jlong stt_inst, jint status )
{
    SpeechToTextAndroid*pThis = (SpeechToTextAndroid*)stt_inst;
    pThis->UpdateStatus( status );
}


#endif // defined(_ANDROID) 