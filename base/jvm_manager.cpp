#ifdef PLATFORM_ANDROID
#include "jvm_manager.h"
#include "macrodef.h"
#include <system/system.h>

JVMManager* JVMManager::self_ = nullptr;

JVMManager:: JVMManager( JavaVM*jvm, jobject jcontext )
{
    jvm_ = jvm;
    JNIEnv *env;
    jint err = jvm_->GetEnv( (void**)&env, JNI_VERSION_1_6 );
    if ( JNI_OK != err )
    {
        logcat("[JVMManager::JVMManager]GetEnv failed:%d", err);
    }
    jctx_ = env->NewGlobalRef( jcontext );
}

JVMManager::~JVMManager()
{
    JNIEnv *env;
    jint err = jvm_->GetEnv( (void**)&env, JNI_VERSION_1_6 );
    if ( JNI_OK != err )
    {
        logcat( "[JVMManager::JVMManager]GetEnv failed:%d", err );
    }
    env->DeleteGlobalRef( jctx_ );
    for (auto & obj:m_objlist)
    {
        env->DeleteGlobalRef( obj.second.first );
        env->DeleteGlobalRef( obj.second.second );

    }
}


JVMManager* JVMManager::GetInstance()
{
    return self_;
}

void JVMManager::Initialize( JavaVM*jvm, jobject jcontext )
{
    ALOGII( "JVMManager::Initialize" );
    if (self_)
    {
        delete self_;
    }

    self_ = new JVMManager( jvm, jcontext );

}

void JVMManager::UnInitialize()
{
      if (self_)
      {
          delete self_;
      }
}

void JVMManager::RegisterClass( int key, const char* cls_name )
{
    auto it = m_objlist.find( key );
    if ( it == m_objlist.end() )
    {
        auto env = JVMManager::GetInstance()->Env();
        if ( !env )
        {
            return;
        }
        jclass cls_ref;

        jclass jcls = env->FindClass( cls_name );
        if ( !jcls )
        {
            return;
        }

        cls_ref = reinterpret_cast<jclass>( env->NewGlobalRef( jcls ) );
        if ( cls_ref )
        {
            m_objlist[key] = { nullptr, cls_ref };
        }
    }
}

jobject JVMManager::Context()
{
    return jctx_;
}

JEnvPtr JVMManager::Env()
{
    bool attach = false;
    JNIEnv* env = nullptr;
    auto jvm = JVMManager::GetInstance()->JVM();
    jint err = jvm->GetEnv( (void**)&env, JNI_VERSION_1_6 );
    if ( env == nullptr || err != JNI_OK )
    {
        if ( err == JNI_EDETACHED )
        {
            err = jvm->AttachCurrentThread( &env, nullptr );
            if ( !env )
            {
                logcat( "can't AttachCurrentThread env=null" );
            }
            else
            {
                attach = true;
            }
        }
    }
    JNIEnvDeleter deleter( jvm, attach );
    return JEnvPtr( env, deleter );
}

void JVMManager::DetachEnv( )
{
    jvm_->DetachCurrentThread();
}

JavaVM* JVMManager::JVM()
{
    return jvm_;
}

jobject JVMManager::GetObject( int key )
{
    auto it = m_objlist.find( key );
    if ( it != m_objlist.end())
    {
        return it->second.first;
    }
    return nullptr;
}

jclass JVMManager::GetClass( int key )
{
    auto it = m_objlist.find( key );
    if ( it != m_objlist.end() )
    {
        return it->second.second;
    }
    return nullptr;
}

void JVMManager::DeleteObject( int key )
{
    auto it = m_objlist.find(key);
    if (it != m_objlist.end())
    {
        Env()->DeleteGlobalRef( it->second.first );
        it->second.first = nullptr;
    }
}


std::pair<jobject,jmethodID> JVMManager::GetMethodID(  int key, const char* name, const char* signature )
{
    auto env = JVMManager::GetInstance()->Env();
    if ( !env )
    {
        return{};
    }
    auto it = m_objlist.find( key );
    if ( it == m_objlist.end() )
    {
        return{};
    }
    jmethodID mid = env->GetMethodID( it->second.second, name, signature );
    return{ it->second.first, mid};
}


#endif //PLATFORM_ANDROID