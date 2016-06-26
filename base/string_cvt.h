#pragma once
#include <cstdlib>
#include <cassert>
#include <locale>
#include <codecvt>


#if defined(WIN32) || defined(_WIN32)
#if _MSC_VER < 1800
#error "should use VS2013 or above version implementation"
#endif
#else
#if __cplusplus < 201103L
#error "should use C++11 implementation"
#endif
#endif//

namespace prudens{
    /*convert unicode string to mbcs*/
    std::string wstombs( const std::wstring& src )
    {
        setlocale( LC_ALL, "chs" );// 设置为中文环境，不然可能会转换失败
        std::vector<char> dst( src.size() * 2 + 1, '\0' );
        size_t count = std::wcstombs( &dst[0], src.c_str(), dst.size() - 1 );
        ASSERT( count > 0 && count < dst.size() );
        setlocale( LC_ALL, "" );
        return std::string( &dst[0] );
    }

    /*convert mbcs to unicode string*/
    std::wstring mbstows( const std::string& src )
    {
        setlocale( LC_ALL, "chs" );// 设置为中文环境，不然可能会转换失败
        std::vector<wchar_t> dst( src.size() + 1, L'\0' );
        size_t count = std::mbstowcs( &dst[0], src.c_str(), dst.size() - 1 );
        ASSERT( count > 0 && count < dst.size() );
        setlocale( LC_ALL, "" );
        return std::wstring( &dst[0] );
    }

    /*convert unicode string to utf8*/
    std::string wstoutf8( const std::wstring& src )
    {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes( src );
    }

    /*convert utf8 string to unicode string*/
    std::wstring utf8tows( const std::string& src_utf8 )
    {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes( src_utf8 );
    }

    /*convert multi byte string(mbcs) to utf8 string*/
    std::string mbstoutf8( const std::string& src_mbcs )
    {
        return wstoutf8( mbstows( src_mbcs ) );
    }

    /*convert utf8 string to mbcs*/
    std::string utf8tombs( const std::string& src_utf8 )
    {
        return wstombs( utf8tows( src_utf8 ) );
    }
}