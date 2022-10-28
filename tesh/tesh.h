#pragma once

#include <FuncList.h>
#include <CodeEdit.h>
#include <TeshAbout.h>
#include <text/ini.h>
#include <text/jassmap.h>

class TESH{
private:
	BOOL next_convert_to_code_dialog;
	BOOL MakeMenu(HMENU hMenu);
	void InitCodeEditRange();
	BOOL SaveEditText();
public:
	TESH();
	~TESH();
	FuncListView FuncListWin;
	CodeEditView CodeEditBox;
	Options _Options;
	TeshAbout _TeshAbout;
	void Resize();
	BOOL Init();
	BOOL InitTriggerEditor(HWND parent, int nCmdShow);
	BOOL InitEditText(HWND parent, int nCmdShow);
	BOOL ShowEditText(HWND parent, int nCmdShow);

	static ini::config_t config, styles;
	/*static jass_map cj_text, bj_text, custom_function_text;
	static jass_map cj_globals_text, bj_globals_text, custom_globals_text;*/
	static HWND WorldEditorMain;
	static HWND WeToolbar;
	static HWND TriggerEditor;
	//static HWND TE_TrigTreeView;
	static HMENU WE_Menu;
	static HMENU TeshMenu;
	static HWND Edit;
	static WNDPROC pWE_WndProc;
	static WNDPROC pTE_WndProc;
	static WNDPROC pEdit_WndProc;

	static BOOL ShowAnyPopCtrlAllSkipIncIme;

	static void ReadJassText();
	static void ReadConfig();
	static void InitConfig();
	static void InitGlobals();
	static void Destroy();

	static void ResumeWorldEditor();
	static void ResumeTriggerEditor();

	static LRESULT CALLBACK CodeEditFocus(HWND hwnd,BOOL enable);
	static LRESULT CALLBACK MenuProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK TE_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK Edit_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK DetourWeWindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	
	static HWND WINAPI DetourWeCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	static BOOL WINAPI DetourWeSetMenu(HWND hWnd, HMENU hMenu);
	static BOOL WINAPI DetourWeShowWindow(HWND hwnd, int nCmdShow);
	static BOOL WINAPI DetourWeSetWindowTextA(HWND hwnd, LPCSTR lpString);
	static BOOL WINAPI DetourWeDestroyWindow(HWND hwnd);
	static BOOL WINAPI DetourWeMessageBoxA(HWND hwnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
	static BOOL WINAPI DetourWePeekMessageA(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);

};
extern TESH Tesh;

//TESH* TESH_Module;