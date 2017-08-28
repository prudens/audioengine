#pragma once
#include <system_error>
enum class audio_error
{
    open_device_failed = 1,
    close_device_failed = 2,
};

class audio_category_impl
    : public std::error_category
{
public:
    virtual const char* name() const _NOEXCEPT
    {
        return "audio";
    }
    virtual std::string message( int ev ) const _NOEXCEPT;
    virtual std::error_condition
        default_error_condition( int ev ) const _NOEXCEPT;
};

namespace std
{
    template <>
    struct is_error_code_enum<audio_error>
        : public true_type{};
}

const std::error_category& audio_category();

std::error_code make_error_code( audio_error e );

std::error_condition make_error_condition( audio_error e );


enum class api_error
{
    low_audio_resource = 1,

};

class api_category_impl
    : public std::error_category
{
public:
    virtual const char* name() const _NOEXCEPT;
    virtual std::string message( int ev ) const;
    virtual bool equivalent(
        const std::error_code& code,
        int condition ) const _NOEXCEPT;
};

const std::error_category& api_category();

std::error_condition make_error_condition( api_error e );

namespace std
{
    template <>
    struct is_error_condition_enum<api_error>
        : public true_type{};
}