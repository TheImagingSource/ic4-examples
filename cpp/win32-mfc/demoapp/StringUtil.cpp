
#include "pch.h"
#include "StringUtil.h"

#include <Windows.h>

std::wstring Win32StringUtil::wstringFromUtf8(const std::string& utf8str)
{
	std::wstring wstr;
	wstr.resize(utf8str.length());

	int numChars = ::MultiByteToWideChar(CP_UTF8, 0, utf8str.data(), static_cast<int>(utf8str.size()), &wstr[0], static_cast<int>(wstr.length()));

	wstr.resize(numChars);
	return wstr;
}