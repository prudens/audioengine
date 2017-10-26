#pragma once
#ifdef _ANDROID
#include <android/log.h>
#ifndef LOG_TAG
#define LOG_TAG    "real_audio_client_test"
#endif
#define ALOGII(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define logE(...) __android_log_print( ANDROID_LOG_ERROR, "android_print_log", __VA_ARGS__);

#define COUT_REDIRECTION_BEGIN      std::ostringstream streambuf; auto cout_buff = std::cout.rdbuf();std::cout.rdbuf( streambuf.rdbuf() );
#define COUT_REDIRECTION_END std::cout.rdbuf( cout_buff );logprint( "%s", streambuf.str().c_str() );
#else
#define COUT_REDIRECTION_BEGIN      std::ostringstream streambuf; auto cout_buff = std::cout.rdbuf();std::cout.rdbuf( streambuf.rdbuf() );
#define COUT_REDIRECTION_END std::cout.rdbuf( cout_buff );logprint( "%s", streambuf.str().c_str() );
#define ALOGII(...)  printf(__VA_ARGS__);
#define logE(...) printf( __VA_ARGS__);
#endif

#ifdef _ANDROID
#include <string>
#include <sstream>
using namespace std;
namespace std
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm;
        stm << n;
        return stm.str();
    }
}
#endif


#ifdef _DEBUG
#define ASSERT(p) assert(p)
#else
#define ASSERT(p) p;
#endif