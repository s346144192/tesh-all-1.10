#pragma once
#include <map>
#include <Options.h>
#include <ScintillaEdit.h>
#include <CtrlFunc.h>
#include <FuncList.h>
#include <base/thread/timer.h>

//#define  WM_SINGLE_CLICK_EDIT WM_USER+124

#define IDR_CODEEDIT_CHID_CTRL 15
#define IDR_CODEEDIT_EDIT 16
#define IDR_CODEEDIT_FIND_BUTTON 17
#define IDR_CODEEDIT_FUNCLIST_BUTTON 18
#define IDR_CODEEDIT_OPTIONS_BUTTON 19
#define IDR_CODEEDIT_FOLDALL_BUTTON 20
#define IDR_CODEEDIT_SYNTAXCHECK_BUTTON 21

#define IDR_FINDTEXT_MAIN 50
#define IDR_FINDTEXT_FOLD 51
#define IDR_FINDTEXT_PUT 52
#define IDR_FINDTEXT_REPPUT 53
#define IDR_FINDTEXT_ENABLECASE 54
#define IDR_FINDTEXT_FULLMATCH 55
#define IDR_FINDTEXT_REGEX 56
#define IDR_FINDTEXT_COUNT 57
#define IDR_FINDTEXT_PREV 58
#define IDR_FINDTEXT_NEXT 59
#define IDR_FINDTEXT_TATGETRANGE 60
#define IDR_FINDTEXT_CLOSE 61
#define IDR_FINDTEXT_REP 62
#define IDR_FINDTEXT_REPALL 63

typedef LRESULT(CALLBACK* EditFocusEventCallback)(HWND hwnd,BOOL enable);


void drawbuttonstate(HWND hwnd, HDC hdc, RECT &rt, HBRUSH primary_hbr, HPEN primary_hpen,BOOL is_focus, BOOL is_down);
struct FindTextCtrl {
private:
	bool buttons_state[8];
public:
	HWND _parent;
	HWND _main;
	HWND _findput;
	HWND _replaceput;
	HWND _find_count;
	controls_t controls;

	splitter_t splitter;

	int _width;
	int _height;

	int _foldheight;

	void Init(HWND parent,int x,int y);
	void setcount(int current, int count);
	bool isshow();
	void show(bool enable, bool enablefold,bool setfocus=false,const char* text=nullptr);
	void setpos(int x,int y);
	void uppos();
	void setsize(int w, int h);
	void setbuttonstate(int _type, bool _enable);
	bool getbuttonstate(int _type);
	void fold(bool enable, bool setfocus=false);
	bool isfold();
	void paint();
	void drawitem(control_t* ctrl);
	
	FindTextCtrl();
	~FindTextCtrl();

	static BindList<HWND, FindTextCtrl*>  classlist;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};


class CodeEditView:public JassEdit
{
public:
	HWND _parent;
	HWND _main;
	HWND _always_fold_all_checkbox;
	HWND _save_scroll_postion_checkbox;

	//Timer _timer;

	controls_t controls;

	FindTextCtrl _findtext;
	int ToolbarHeight;

	int line_num_width;

	bool enable_auto_complete;
	bool enable_calltip;
	BOOL enable_autofillspace;
	BOOL enable_alwaysfoldall;

	BOOL enable_save_scroll_postion;

	int scroll_pos;
	BOOL always_fold;

	int _edit_state;
	BOOL _edit_once_change;
	BOOL _edit_focus;

	//ÁíÒ»¸ö´°¿Ú
	FuncListView* FuncList;
	Options* _Options;

	CodeEditView();
	~CodeEditView();
	
	EditFocusEventCallback EditFocusCallback;

	
	BOOL SetEditPutText(LPCSTR lpString);
	LPSTR GetEditPutText();

	void SetConfig(const char* node, const char* keyword, const char* value);
	void SetConfig(const char* node, const char* keyword, int value);
	int GetConfigInt(const char* node, const char* keyword, int default_value = 0);
	void ShowFoldExt(bool enable);
	void ShowLineNumberExt(bool enable);
	void EnableAutoCompleteExt(bool enable);
	void EnableCallTipExt(bool enable);
	void EnableZoomLevelExt(bool enable);
	void SetTabLineExt(BOOL enable);
	void SetAutoFillSpaceExt(BOOL enable);

	void EnableAlwaysFoldAllExt(BOOL enable);
	void EnableSaveSrcollPosExt(BOOL enable);

	void EnableUTF8Ext(BOOL enable);
	void EnableD2dDrawExt(BOOL enable);
	void EnableEnableAntiAliasExt(int type);
	void SetEditFontExt(const char* name, int size);
	void SetLineNumFontExt(const char* name, int size);

	void AutoAlwaysFoldAll(BOOL IsInitFold=FALSE);

	int LoadCompleteList(const char* word, int wn, size_t max, bool isluaword);
	std::string GetCallTip(std::string function_name);
	
	void ShowCompleteList();
	void CallTipHighlight();
	const char* GetWordText(int pos, int startpos);
	void ShowCallTip();
	bool ShowParentCallTip();
	bool SearchPrevCallTip(int pos);

	void CallCtrlF();
	void CallCtrlH();
	BOOL FindTextEvent(WORD ctrlid, WORD eventid);
	void ClickControl(int id);
	LRESULT EditEvent(HWND hwnd, SCNotification* notify);
	LRESULT TriggerCtrlEvent(HWND hwnd, int ctrlid, int eventid);
	

	void Resize();
	void MovePos(int x, int y);
	void SetSize(int w, int h);
	BOOL Init(HWND parent, Options* Options);
	BOOL Destroy();

	//static void TimerSyncEditMouseClick(uint32_t pCodeEditView, uint32_t args2);
	
	static BindList<HWND,CodeEditView*> classlist;
	static void __stdcall MessageNotify(sptr_t ptr, SCNotification* notify);
	static void __stdcall MessageCommand(sptr_t ptr, int chidctrlid, int eventid);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

