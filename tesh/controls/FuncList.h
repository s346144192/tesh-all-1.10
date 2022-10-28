#pragma once
#include <ScintillaEdit.h>
#include <dll_info.h>
#include <CtrlFunc.h>
#include <Splitter.h>

#include <text/jassmap.h>

#define IDR_FUNCLIST_SEARCH_INPUT 0x10
#define IDR_FUNCLIST_CB_NATIVES 0x11
#define IDR_FUNCLIST_CB_BJS 0x12
#define IDR_FUNCLIST_CB_CUSTOM 0x13
#define IDR_FUNCLIST_SEL_FUNCTIONS 0x14
#define IDR_FUNCLIST_SEL_CONSTANTS 0x15
#define IDR_FUNCLIST_LIST 0x16
#define IDR_FUNCLIST_CODEBOX 0x17
#define IDR_FUNCLIST_BTN_UP_DOWN 0x18
#define IDR_FUNCLIST_SPLITTER 0x19


char* stristr(const char* s1, const char* s2);

class FuncListView
{
public:
	HWND _parent;
	HWND _main;
	HWND _textbox_search_input;
	HWND _checkbox_natives;
	HWND _checkbox_BJs;
	HWND _checkbox_custom;
	HWND _select_functions;
	HWND _select_constants;
	HWND btn_up_down_show;
	HWND func_list;
	JassEdit _codebox;
	splitter_t splitter;

	int btn_up_down_winstate;
	FuncListView();
	~FuncListView();
	void SetFuncText(const char* text);
	void LoadListFuncTextByIndex(int i);
	void LoadCurSelFuncText();
	int FindFunc(const char* name);
	void SetCurSel(int i);
	void SetFindCurSel(const char* name);
	void ResetFuncList();
	void LoadFuncList(const char* match_str, jass_map& jass_text,bool gl_constant, bool reset = true);
	void LoadFuncList();

	void TriggerCtrlEvent(HWND hwnd, WORD ctrl, WORD eventid);
	BOOL Show(int ishow);
	void UpPoint();
	void Resize();
	BOOL Init(HWND parent);

	static std::string get_func_codetext(const char* name);
	static void splitter_up(HWND parent,HWND hwnd);

	static BaseMap<HWND, FuncListView*> classlist;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};