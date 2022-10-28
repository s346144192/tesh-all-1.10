#include <stdio.h>
#include <ObjBase.h>
#include <dll_info.h>

	windows_ver win_ver;
	windows_ver::windows_ver()
	{
		num = 0; major = 0; minor = 0; build = 0;
		//判断是否为64位系统
		SYSTEM_INFO cur_system_info;
		GetNativeSystemInfo(&cur_system_info);
		//WORD system_str = cur_system_info.wProcessorArchitecture;

		DWORD dwVersion = 0;
		HMODULE hModNtdll = NULL;
		if ((hModNtdll = LoadLibrary(TEXT("ntdll.dll"))))
		{
			typedef void (WINAPI *RtlGetNtVersionNumbers)(DWORD*, DWORD*, DWORD*);
			RtlGetNtVersionNumbers pfRtlGetNtVersionNumbers;
			pfRtlGetNtVersionNumbers = (RtlGetNtVersionNumbers)GetProcAddress(hModNtdll, "RtlGetNtVersionNumbers");
			if (pfRtlGetNtVersionNumbers)
			{
				pfRtlGetNtVersionNumbers(&major, &minor, &build);
				build &= 0xffff;
			}
			FreeLibrary(hModNtdll);

			if (major == 6 && minor == 0) {
				num = 6;
			}
			else if (major == 6 && minor == 1) {
				num = 7;
			}
			else if (major == 6 && minor >= 2) {
				num = 8;
			}
			else if (major == 10) {
				num = 10;
			}
			else {
				num = major;
			}
		}
	}
	windows_ver::~windows_ver()
	{
		num = 0; major = 0; minor = 0; build = 0;
	}

	com_loader_c::com_loader_c() {
		hr = RPC_E_CHANGED_MODE;
	}
	com_loader_c::~com_loader_c() {
		if (!(FAILED(hr))) {
			::CoUninitialize();
		}
		
	}
	bool com_loader_c::init() {
		if (FAILED(hr)) {
			hr = ::CoInitialize(NULL);
		}
		return isinit();
	}
	bool com_loader_c::isinit() {
		if (FAILED(hr)) {
			return FALSE;
		}
		return TRUE;
	}

	dll_info dllself;

	dll_info::dll_info() {
		load(GetModuleHandleA(NULL));
	}
	dll_info::dll_info(HMODULE module) {
		load(module);
	}
	void dll_info::operator()(HMODULE module) {
		load(module);
	}
	void dll_info::load(HMODULE module) {
		_module = module;
		ZeroMemory(name, sizeof name);
		ZeroMemory(path, sizeof path);
		if (!_module) {
			return;
		}
		size_t len = GetModuleFileNameA(_module, path, MAX_PATH);
		char* air = strrchr(path, '\\');
		if (air > 0) {
			if (path + len > air + 1) {
				memcpy(name, air + 1, (size_t)path + len - ((size_t)air + 1));
			}
			ZeroMemory(air, sizeof(path) - (air - path));
		}
	}
	int dll_info::getfilepath(const char* name, char* newpath, int length)
	{
		if (!*path || !name || !newpath)
			return -1;
		if (*name == '\\')
			++name;
		return sprintf_s(newpath, length, "%s%s%s", path, "\\", name);
	}
	std::string dll_info::getfilepath(const char* name) {
		char fullpath[MAX_PATH] = { 0 };
		std::string str;
		getfilepath(name, fullpath,sizeof(fullpath));
		str.assign(fullpath);
		return str;
	}
	void dll_info::getfilepath(std::string& str, const char* name) {
		str = getfilepath(name);
	}

	char* GetDllPath()
	{
		static char path[MAX_PATH] = { 0 };
		GetModuleFileNameA(dllself._module, path, MAX_PATH);
		char* c = strrchr(path, '\\');
		if (c > 0) {
			*c = 0;
		}
		return dllself.path;
	}
	int LinkDllPath(const char* name, char* newpath, int length)
	{
		return dllself.getfilepath(name, newpath, length);
	}

	//DLLMain更改
	void debug(const char *fmt, ...)
	{
		//不定参数格式化
		va_list argptr;
		va_start(argptr, fmt);
		char buffer[4096] = { 0 };
		int cnt = vsprintf_s(buffer, fmt, argptr);
		va_end(argptr);
		buffer[sizeof(buffer) - 1] = 0;
		OutputDebugStringA(buffer);
	}


	

	//int gettimeofday(struct timeval *tp, void *tzp);

