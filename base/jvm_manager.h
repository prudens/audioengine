#pragma once
#ifdef PLATFORM_ANDOIRD
#include <jni.h>
#include <map>
#include <system/system.h>
#include "macrodef.h"

class JNIEnvDeleter
{
public:
    JNIEnvDeleter( JavaVM*jvm, bool attach )
    :jvm_(jvm)
    ,attach_(attach)
    {

    }
    void operator()( JNIEnv*p )
    {
        if (attach_)
        {
            jvm_->DetachCurrentThread();
        }
    }
private:
    JavaVM* jvm_;
    bool attach_;
};
typedef std::unique_ptr<JNIEnv, JNIEnvDeleter>   JEnvPtr;
class JVMManager
{
public:
    enum {
        JAVA_CLS_AudioToText = 1,
        JAVA_CLS_ContextRef  = 2,
        JAVA_CLS_AudioDevice = 3,
        JAVA_CLS_AudioDeviceListener = 4,
    };
    ~JVMManager();
    static JVMManager* GetInstance();
    static void Initialize( JavaVM*jvm ,jobject jcontext);
    static void UnInitialize();
    void RegisterClass(int key, const char* cls_name);
    jobject Context();
    JEnvPtr Env();
    void    DetachEnv();
    JavaVM* JVM();
    template<typename ...Args>
    jobject NewObject( int key, const char* signature, Args const & ... args )
    {
        auto it = m_objlist.find( key );
        if ( it == m_objlist.end()
             || !it->second.second)
        {
            return nullptr;
        }
        jclass cls = it->second.second;

        auto env = JVMManager::GetInstance()->Env();
        if ( !env )
        {
            return nullptr;
        }

        jmethodID mid_init = env->GetMethodID( cls, "<init>", signature );
        jobject obj = env->NewObject( cls, mid_init, args... );
        jobject obj_ref = env->NewGlobalRef( obj );
        env->DeleteLocalRef( obj );
      
        it->second.first = obj_ref;
        return obj_ref;
    }
    jobject GetObject( int key );
    jclass  GetClass( int key );
    void DeleteObject(int key);
    std::pair<jobject,jmethodID> GetMethodID( int key, const char* name, const char* signature );
    template <typename ... Args>
    void CallVoidMethod( int key, const char* name, const char* signature, Args const & ... args )
    {
        auto obj_mid = GetMethodID( key, name, signature );
        auto env = self_->Env();
        env->CallVoidMethod( obj_mid.first, obj_mid.second, args... );
    }

    template <typename ... Args>
    jboolean CallBooleanMethod( int key, const char* name, const char* signature, Args const & ... args )
    {
        auto obj_mid = GetMethodID( key, name, signature );
        auto env = self_->Env();
        return env->CallBooleanMethod( obj_mid.first, obj_mid.second, args... );
    }

    template <typename ... Args>
    int CallIntMethod( int key, const char* name, const char* signature, Args const & ... args )
    {
        auto obj_mid = GetMethodID( key, name, signature );
        auto env = self_->Env();
        return env->CallIntMethod( obj_mid.first, obj_mid.second, args... );
    }

    template <typename ... Args>
    jobject CallObjectMethod( int key, const char* name, const char* signature, Args const & ... args )
    {
        auto obj_mid = GetMethodID( key, name, signature );
        auto env = self_->Env();
        return env->CallObjectMethod( obj_mid.first, obj_mid.second, args... );
    }

    template<typename...Args>
    jobject CallStaticObjectMethod( int key, const char* name, const char* signature, Args const & ... args )
    {
        jclass cls = self_->GetClass( key );
        auto env = self_->Env();
        jmethodID mid = env->GetStaticMethodID( cls, name, signature );
        return env->CallStaticObjectMethod( cls, mid, args... );
    }
private:
    JVMManager( JavaVM*jvm, jobject jcontext );
    JVMManager( const JVMManager& ) = delete;
    JVMManager& operator=( const JVMManager& ) = delete;
    static JVMManager* self_;
    JavaVM* jvm_ = nullptr;
    jobject jctx_ = nullptr;
    std::map<int,std::pair<jobject,jclass> > m_objlist;
};
#endif // PLATFORM_ANDROID