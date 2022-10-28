#pragma once

#include <windows.h>
#include <CtrlFunc.h>
class TeshAbout {
public:
	HWND _hwnd;
	HWND _tip;
	HWND _description;
	HWND _email_tip; HWND _email_addr;
	HWND _developer;
	HFONT email_hfont;
	TeshAbout() {
		WinClass.Register("TeshAboutWin", WndProc);
	}
	~TeshAbout() {
		WinClass.UnRegister("TeshAboutWin");
	}
	void Show(bool enable);
	void Init(HWND parent,const char* tip, const char* description, const char* email_addr, const char* developer);
	static BaseMap<HWND, TeshAbout*> classlist;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};