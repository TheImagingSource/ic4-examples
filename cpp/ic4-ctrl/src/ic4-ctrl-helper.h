
#pragma once

#include <stdlib.h> // getenv, setenv
#include <string>

#if defined _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <fmt/core.h>

template<class ... Targs>
void print(fmt::format_string<Targs...> fmt, Targs&& ... args)
{
	fmt::print(fmt, std::forward<Targs>(args)...);
}

template<class ... Targs>
void print(int offset, fmt::format_string<Targs...> fmt, Targs&& ... args)
{
	for (int i = 0; i < offset; ++i) {
		fmt::print("    ");
	}
	fmt::print(fmt, std::forward<Targs>(args)...);
}

namespace helper
{
    template<typename T>
    auto from_chars_helper( const std::string& str, T& value ) -> bool = delete;

    template<>
    inline auto from_chars_helper<int64_t>( const std::string& str, int64_t& value ) -> bool
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
	inline auto from_chars_helper<double>( const std::string& str, double& value ) -> bool
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



#if defined _WIN32
    inline void set_env_var( std::string env_name, std::string value )
    {
        ::SetEnvironmentVariableA( env_name.c_str(), value.c_str() );
    }

    inline auto get_env_var( std::string env_name ) -> std::string
    {
        char* buf = nullptr;
        size_t buf_len = 0;
        auto err = ::_dupenv_s( &buf, &buf_len, env_name.c_str() );
        if( err || buf == nullptr ) { 
            return {};
        }
        std::string str{ buf, buf_len };
        ::free( buf );
        return str;
    }
#else
    inline void set_env_var( std::string env_name, std::string value )
    {
        ::setenv( env_name.c_str(), value.c_str(), 1 );
    }

    inline auto get_env_var( std::string env_name ) -> std::string
    {
        auto ptr = ::getenv( env_name.c_str() );
        if( !ptr ) {
            return {};
        }
        return std::string{ ptr };
    }
#endif

}
