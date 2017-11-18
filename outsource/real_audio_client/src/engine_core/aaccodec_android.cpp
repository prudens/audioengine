#ifdef _ANDROID
#include <jni.h>
#include "../../include/BotechAudio.h"
#include "../../engine/jvm_manager.h"
#include "../include/codec_converter.h"
#include <algorithm>
#include <vector>
#include <mutex>
#define    LOGCAT(...)  __android_log_print(ANDROID_LOG_INFO,"AudioEngine",__VA_ARGS__)
#ifdef __cplusplus
extern "C" {
#endif
    /*
    * Class:     com_snail_codec_AACDecoder
    * Method:    CacheDirectBufferAddress
    * Signature: (JLjava/nio/ByteBuffer;)V
    */
    JNIEXPORT void JNICALL Java_com_snail_codec_AACDecoder_CacheDirectBufferAddress
        ( JNIEnv *, jobject, jlong, jobject );
    
    /*
    * Class:     com_snail_codec_AACEncoder
    * Method:    UpdateMediaFormat
    * Signature: (JI)V
    */
    JNIEXPORT void JNICALL Java_com_snail_codec_AACDecoder_UpdateMediaFormat
        (JNIEnv *, jobject, jlong, jint);
    
    /*
    * Class:     com_snail_codec_AACEncoder
    * Method:    CacheDirectBufferAddress
    * Signature: (JLjava/nio/ByteBuffer;)V
    */
    JNIEXPORT void JNICALL Java_com_snail_codec_AACEncoder_CacheDirectBufferAddress
        (JNIEnv *, jobject, jlong, jobject);
    
#ifdef __cplusplus
}
#endif
struct AACSetupData 
{
    int smaplerate;
    int channel;
    int16_t profile;
    int16_t csd;
};
std::vector<AACSetupData> aac_setup_data_list;
std::mutex  aac_setup_data_list_lock;
using namespace bodtech::audio;
class AACEncoderAndroid:public CodecConverter
{
public:
    AACEncoderAndroid()
    {
        JVMManager* jvm = JVMManager::GetInstance();
        m_aac_converter_id = jvm->NewObject( JAVA_CLS_AACEncoder,"(J)V",(jlong)this);
    }

    virtual ~AACEncoderAndroid()
    {
        JVMManager::GetInstance()->CallVoidMethod( m_aac_converter_id, "stop", "()V" );
        JVMManager::GetInstance()->DeleteObject(m_aac_converter_id);
    }

    bool Init(int aacObjectType, int samplerate, int channel, int bitrate )
    {
        JVMManager* jvm = JVMManager::GetInstance();
        bool bsuc = jvm->CallBooleanMethod( m_aac_converter_id, "start", "(IIII)Z", samplerate, channel, bitrate, aacObjectType );
//        if (bsuc)
        {
            if ( m_direct_buffer)
            {
                aac_setup_data_list_lock.lock();
                int16_t t = 0;
                char* p = (char*)m_direct_buffer;
                t = *p << 8;
                t += *( p+1);
                auto it = std::find_if( aac_setup_data_list.begin(), aac_setup_data_list.end(), [=] ( const AACSetupData& a )
                {
                    return a.channel == channel && a.smaplerate == samplerate && a.profile == aacObjectType;
                } );

                if ( it != aac_setup_data_list.end() )
                {
                    it->csd = t;
                }
                else
                {
                    AACSetupData data;
                    data.channel = channel;
                    data.smaplerate = samplerate;
                    data.profile = aacObjectType;
                    data.csd = t;
                    aac_setup_data_list.push_back( data );
                }
                aac_setup_data_list_lock.unlock();
            }
        }
        return bsuc;
    }

    virtual int Process(const void* inputData, size_t inputSize, void* outputData, size_t& outputSize )
    {
        if ( m_direct_buffer == nullptr || inputSize > m_length || outputData == nullptr || outputSize == 0 )
        {
            return -1;
        }
        if ( inputData  && inputSize>0 )
        {
            memcpy( m_direct_buffer, inputData, inputSize );
        }

        JVMManager* jvm = JVMManager::GetInstance();
        size_t size = jvm->CallIntMethod( m_aac_converter_id, "encode", "(I)I", inputSize );
        if ( size > outputSize )
        {
            return -1;
        }
        if ( size > 0 )
        {
            memcpy( outputData, m_direct_buffer, size );
        }
        outputSize = size;
        return 0;
    }
    virtual void Destroy()
    {
        delete this;
    }
    virtual int  ErrorCode()
    {
        return 0;//fix me
    }
    void SetDirectBuffer( void* direct_buffer, size_t length )
    {
        m_direct_buffer = direct_buffer;
        m_length = length;
    }
private:
    int m_aac_converter_id = 0;
    void* m_direct_buffer = nullptr;
    size_t m_length = 0;
};

class AACDecoderAndroid :public CodecConverter
{
public:
    AACDecoderAndroid()
    {
        JVMManager* jvm = JVMManager::GetInstance();
        m_aac_converter_id = jvm->NewObject( JAVA_CLS_AACDecoder, "(J)V", (jlong)this );//this指针必须强转，否则不会正确传值
    }

    virtual ~AACDecoderAndroid()
    {
        JVMManager::GetInstance()->CallVoidMethod( m_aac_converter_id,"stop","()V" );
        JVMManager::GetInstance()->DeleteObject( m_aac_converter_id );
    }

    bool Init( int aacObjectType, int samplerate, int channel )
    {
        int16_t csd = 0;
        {
            aac_setup_data_list_lock.lock();
            auto it = std::find_if( aac_setup_data_list.begin(), aac_setup_data_list.end(), [=] ( const AACSetupData& a )
            {
                return a.channel == channel && a.smaplerate == samplerate && a.profile == aacObjectType;
            } );
            if ( it != aac_setup_data_list.end() )
            {
                csd = it->csd;
            }
            aac_setup_data_list_lock.unlock();
        }
        if (csd == 0)
        {
            AACEncoderAndroid encoder;
            encoder.Init(aacObjectType, samplerate, channel, 32000 );
            {
                aac_setup_data_list_lock.lock();
                auto it = std::find_if( aac_setup_data_list.begin(), aac_setup_data_list.end(), [=] ( const AACSetupData& a )
                {
                    return a.channel == channel && a.smaplerate == samplerate && a.profile == aacObjectType;
                } );
                if ( it != aac_setup_data_list.end() )
                {
                    csd = it->csd;
                }
                aac_setup_data_list_lock.unlock();
            }
        }
        JVMManager* jvm = JVMManager::GetInstance();
        bool bsuc = jvm->CallBooleanMethod( m_aac_converter_id, "start", "(IIII)Z", samplerate, channel, (int)csd,aacObjectType );
        return bsuc;
    }

    virtual int Process(const void* inputData, size_t inputSize, void* outputData, size_t& outputSize )
    {
        if (inputSize == 0)
        {
            return -1;
        }
        if ( m_direct_buffer == nullptr || inputSize > m_length || outputData == nullptr || outputSize == 0)
        {
            return -1;
        }
        JVMManager* jvm = JVMManager::GetInstance();
        if ( inputData && inputSize > 0 )
        {
            memcpy( m_direct_buffer, inputData, inputSize );
        }

        size_t size = jvm->CallIntMethod( m_aac_converter_id, "decode", "(I)I", inputSize );
        if (size > outputSize )
        {
            return -1;
        }
        if( m_dec_channel == 1 && outputSize >= size * 2)
        {
            const int16_t* pSrc = (const int16_t*)m_direct_buffer;
            int16_t* pDst = (int16_t*)outputData;
            for ( size_t i = 0; i < size/2; i++ )
            {
                pDst[i*2] = pSrc[i];
                pDst[i*2 + 1] = pSrc[i];
            }
            size *= 2;
        }
        else if ( size > 0 )
        {
            memcpy( outputData, m_direct_buffer, size );
        }
        
        outputSize = size;
        return 0;
    }

    virtual void Destroy()
    {
        delete this;
    }
    virtual int  ErrorCode()
    {
        return 0;//fix me
    }
    void SetDirectBuffer( void* direct_buffer, size_t length )
    {
        m_direct_buffer = direct_buffer;
        m_length = length;
    }

    void UpdateMediaFormat(int channel)
    {
        m_dec_channel = channel;
    }
private:
    int m_aac_converter_id = 0;
    void* m_direct_buffer = nullptr;
    size_t m_length = 0;
    int m_dec_channel = 2;
};


JNIEXPORT void JNICALL Java_com_snail_codec_AACEncoder_CacheDirectBufferAddress
( JNIEnv *env, jobject, jlong self, jobject obj )
{
    void* byte_buffer = env->GetDirectBufferAddress( obj );
    size_t length = (size_t)env->GetDirectBufferCapacity( obj );
    AACEncoderAndroid* pThis = (AACEncoderAndroid*)self;
    if ( pThis )
    {
        pThis->SetDirectBuffer( byte_buffer, length );
    }
}

JNIEXPORT void JNICALL Java_com_snail_codec_AACDecoder_CacheDirectBufferAddress
( JNIEnv *env, jobject, jlong self, jobject obj )
{
    void* byte_buffer = env->GetDirectBufferAddress( obj );
    size_t length = (size_t)env->GetDirectBufferCapacity( obj );
    AACDecoderAndroid* pThis = (AACDecoderAndroid*)self;
    if ( pThis )
    {
        pThis->SetDirectBuffer( byte_buffer, length );
    }
}

JNIEXPORT void JNICALL Java_com_snail_codec_AACDecoder_UpdateMediaFormat
( JNIEnv *, jobject, jlong self, jint  channel)
{
    AACDecoderAndroid* pThis = (AACDecoderAndroid*)self;
    if (pThis)
    {
        pThis->UpdateMediaFormat( channel);
    }
}
#endif