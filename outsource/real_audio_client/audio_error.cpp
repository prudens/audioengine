#include "audio_error.h"
struct audio_error_table
{
    audio_error err_code;
    const char* name;
};
static const audio_error_table audio_err_tab[] =
{
    audio_error::open_device_failed, "Open audio device failed",
    audio_error::close_device_failed, "Close audio device failed",
};

std::string audio_category_impl::message( int ev ) const _NOEXCEPT
{
    const audio_error_table *ptr = &audio_err_tab[0];
    for ( ; ptr->name != 0; ++ptr )
        if ( (int)ptr->err_code == ev )
            return ( ptr->name );
    return "Unknown audio error";
}

std::error_condition audio_category_impl::default_error_condition( int ev ) const _NOEXCEPT
{
    return std::error_condition( ev, *this );
}


std::error_condition make_error_condition( audio_error e )
{
    return std::error_condition(
        static_cast<int>( e ),
        audio_category() );
}

std::error_condition make_error_condition( api_error e )
{
    return std::error_condition(
        static_cast<int>( e ),
        api_category() );
}

std::error_code make_error_code( audio_error e )
{
    return std::error_code(
        static_cast<int>( e ),
        audio_category() );
}



///////////////////////////////////////////////////////////
const char* api_category_impl::name() const _NOEXCEPT
{
    return "api";
}

std::string api_category_impl::message( int ev ) const
{
    switch ( ev )
    {
    case (int)api_error::low_audio_resource:
        return "Low system resources";
    default:
        return "Unknown api error";
    }
}

bool api_category_impl::equivalent(
    const std::error_code& code,
    int condition ) const _NOEXCEPT
{
    switch ( condition )
    {
    case (int)api_error::low_audio_resource:
        return code == audio_error::open_device_failed
            || code == audio_error::close_device_failed;
    default:
        return false;
    }
}


const std::error_category& audio_category()
{
    static audio_category_impl instance;
    return instance;
}

const std::error_category& api_category()
{
    static api_category_impl instance;
    return instance;
}