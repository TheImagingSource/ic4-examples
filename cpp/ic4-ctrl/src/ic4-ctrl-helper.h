
#pragma once

#include <stdlib.h> // getenv, setenv
#include <string>

#if defined WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace helper
{
    template<typename T>
    auto from_chars_helper( const std::string& str, T& value ) -> bool = delete;

    template<>
    auto from_chars_helper<int64_t>( const std::string& str, int64_t& value ) -> bool
    {
        try
        {
            value = std::stoll( str, nullptr, 10 );
            return true;
        }
        catch( const std::exception& /*ex*/ )
        {
            return false;
        }
    }

    template<>
    auto from_chars_helper<double>( const std::string& str, double& value ) -> bool
    {
        try
        {
            value = std::stod( str, nullptr );
            return true;
        }
        catch( const std::exception& /*ex*/ )
        {
            return false;
        }
    }


    inline auto get_env_var( std::string env_name ) -> std::string
    {
        auto ptr = ::getenv( env_name.c_str() );
        if( !ptr ) {
            return {};
        }
        return std::string{ ptr };
    }

#if defined WIN32
    inline void set_env_var( std::string env_name, std::string value )
    {
        ::SetEnvironmentVariableA( env_name.c_str(), value.c_str() );
    }
#else
    inline void set_env_var( std::string env_name, std::string value )
    {
        ::setenv( env_name.c_str(), value.c_str(), 1 );
    }
#endif

}
