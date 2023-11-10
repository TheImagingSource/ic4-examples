#pragma once

#include <string>

namespace Win32StringUtil
{
	std::wstring wstringFromUtf8(const std::string& str);
}