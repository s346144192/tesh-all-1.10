#pragma once
#include <string>
#include <windows.h>

class windows_ver {
public:
	DWORD num;
	DWORD major;
	DWORD minor;
	DWORD build;
	windows_ver();
	~windows_ver();
};
extern windows_ver win_ver;

static class com_loader_c {
private:
	HRESULT hr;
public:
	com_loader_c();
	~com_loader_c();
	bool init();
	bool isinit();

}com_loader;

class dll_info {
private:
	void load(HMODULE module);
public:
	HMODULE _module;
	char name[MAX_PATH];
	char path[MAX_PATH];

	dll_info();
	dll_info(HMODULE module);
	void operator()(HMODULE module);
	int getfilepath(const char* name, char* newpath, int length);
	std::string getfilepath(const char* name);
	void getfilepath(std::string& str,const char* name);

};
extern dll_info dllself;

char* GetDllPath();
int LinkDllPath(const char* name, char* newpath, int length);

void debug(const char *fmt, ...);
