#include <windows.h>
#include <base/encode/convert_utf_8.h>

namespace base {
	//UTF-8到GB2312的转换
	char* U2G(const char* utf8)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[len + 1];
		memset(wstr, 0, len + 1);
		MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
		len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
		char* str = new char[len + 1];
		memset(str, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
		if (wstr) delete[] wstr;
		return str;
	}
	int U2G(const char* utf8, char gb2312[], size_t length)
	{
		if (!gb2312) {
			return 0;
		}
		if (!utf8) {
			memset(gb2312, 0, length);
		}
		int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[len + 1];
		memset(wstr, 0, (len + 1) * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
		len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
		memset(gb2312, 0, length);
		if (length > len) { length = len; }
		int rt = 0;
		if (length > 0) {
			rt=WideCharToMultiByte(CP_ACP, 0, wstr, -1, gb2312, length, NULL, NULL);
		}
		//gb2312[length - 1] = 0;
		if (wstr) delete[] wstr;
		return rt;
	}
	BOOL C_U2G(const char* utf8, char gb2312[], size_t length)
	{
		if (utf8) {
			if (base::is_utf8(utf8)) {
				base::U2G(utf8, gb2312, length);
			}
			else {
				sprintf_s(gb2312, length, "%s", utf8);
			}
		}
		else {
			memset(gb2312, 0, length);
		}
		return TRUE;
	}
	//GB2312到UTF-8的转换
	char* G2U(const char* gb2312)
	{
		int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[len + 1];
		memset(wstr, 0, len + 1);
		MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
		char* str = new char[len + 1];
		memset(str, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
		if (wstr) delete[] wstr;
		return str;
	}

	int G2U(const char* gb2312, char utf8[], size_t length)
	{
		if (!utf8) {
			return 0;
		}
		if (!gb2312) {
			memset(utf8, 0, length);
		}
		int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[len + 1];
		memset(wstr, 0, (len + 1)*sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
		char* str = new char[len + 1];
		memset(str, 0, len + 1);
		int rt = 0;
		if (length > 0) {
			rt = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, length, NULL, NULL);
		}
		if (wstr) delete[] wstr;
		return rt;
	}
	std::wstring CharToWchar(const char* c, size_t m_encode)
	{
		std::wstring str;
		int len = MultiByteToWideChar(m_encode, 0, c, strlen(c), NULL, 0);
		wchar_t*	m_wchar = new wchar_t[len + 1];
		MultiByteToWideChar(m_encode, 0, c, strlen(c), m_wchar, len);
		m_wchar[len] = '\0';
		str = m_wchar;
		delete m_wchar;
		return str;
	}

	std::string WcharToChar(const wchar_t* wp, size_t m_encode)
	{
		std::string str;
		int len = WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
		char	*m_char = new char[len + 1];
		WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
		m_char[len] = '\0';
		str = m_char;
		delete m_char;
		return str;
	}

}