#pragma once

#include <string>

#include <base/util/unicode.h>

#ifndef BASE_ENCODE_CONVERT_UTF_8
#define BASE_ENCODE_CONVERT_UTF_8

namespace base {
	//UTF-8µ½GB2312µÄ×ª»»
	char* U2G(const char* utf8);

	int U2G(const char* utf8, char gb2312[], size_t length);

	int C_U2G(const char* utf8, char gb2312[], size_t length);

	char* G2U(const char* gb2312);

	int G2U(const char* gb2312, char utf8[], size_t length);

	std::wstring CharToWchar(const char* c, size_t m_encode = 0);

	std::string WcharToChar(const wchar_t* wp, size_t m_encode = 0);

}


#endif
