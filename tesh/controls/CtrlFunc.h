#pragma once
#include <windows.h>
#include <stdint.h>
#include <map>
#include <vector>
#include <Commdlg.h> 
#include <Resource.h>
#include <dll_info.h>

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0) //必要的

#define WM_USER_CONTROL_UP WM_USER+123

typedef LRESULT(CALLBACK WndProcType)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//DDX_Control(pDX, IDC_EDIT1, m_Edit1);//关联一个控件

extern uint32_t double_click_time;

template <class T1, class T2>
class BaseMap {
private:
	std::map<T1, T2> data;
public:
	BOOL add(T1 name, T2 _class) {
		data[name] = _class;
		return TRUE;
	}
	T2 get(T1 name) {
		auto it = data.find(name);
		if (it != data.end())
		{
			return (*it).second;
		}
		return (T2)NULL;
	}
};
template <typename T1, typename T2>
class databind_t {
public:
	T1 first;
	T2 second;
	databind_t(T1 _first, T2 _second) {
		first = _first; second = _second;
	}
};
template <typename T1>
class BaseList {
public:
	std::vector<T1> data;
	T1& add(T1& value) {
		data.push_back(value);
		return data.back();
	}
	auto begin() {
		return data.begin();
	}
	auto end() {
		return data.end();
	}
	T1& at(int i) {
		return data.at(i);
	}
};
template <typename T1, typename T2>
class BindList:public BaseList<databind_t<T1, T2>> {
public:
	databind_t<T1, T2>& add(T1 _first, T2 _second) {
		data.push_back(databind_t<T1, T2>(_first, _second));
		return data.back();
	}
	int getid(T1 name) {
		size_t count = data.size();
		for (size_t i=0;i< count;i++) {
			if (data[i].first ==name) {return i;}
		}
		return -1;
	}
};


struct control_t {
public:
	static control_t* focus;
	static control_t* lastdownfocus;
	int id;
	int level;
	HWND parent;
	RECT rt;
	HFONT hfont;
	const char* text;
	BOOL isdown;
	BOOL hidefocus;
	BOOL singlesel;
	control_t(const char* _text, HWND _parent, int _id, RECT _rt, BOOL _hidefocus=FALSE, BOOL _singlesel = FALSE) {
		parent = _parent; hfont = NULL; text = _text; id = _id; rt = _rt;
		hidefocus = _hidefocus; singlesel = _singlesel; isdown = FALSE;
		rt.right = rt.left + rt.right; rt.bottom = rt.top + rt.bottom;
	}
	void setpos(int x, int y) {
		int w = rt.right - rt.left; int h= rt.bottom - rt.top;
		rt.left = x; rt.top = y;
		rt.right = rt.left + w; rt.bottom = rt.top + h;
	}
	void setsize(int w, int h) {
		rt.right = rt.left + w; rt.bottom = rt.top + h;
	}
	BOOL inrange(int x,int y) {
		return x >= rt.left && x <= rt.right && y >= rt.top && y <= rt.bottom;
	}
	BOOL isfocus() {
		return this==focus;
	}
	static void setfocus(control_t* ctrl) {
		focus = ctrl;
	}
	static control_t* focusleave(int x, int y) {
		if (focus) {
			control_t* _focus= focus;
			if (!focus->inrange(x,y)) {
				setfocus(NULL);
				return _focus;
			}
		}
		return NULL;
	}
};

class controls_t {
public:
	BaseList<control_t> controls;
	control_t* front() {
		if (controls.data.empty())
			return NULL;
		return &controls.data.front();
	}
	control_t* back() {
		if (controls.data.empty())
			return NULL;
		return &controls.data.back();
	}
	control_t* at(size_t idx) {
		if (idx>=0 && idx< controls.data.size())
			return &controls.data.at(idx);
		return NULL;
	}
	control_t* get(int id) {
		size_t count = controls.data.size();
		for (size_t i = 0; i < count; i++) {
			if (controls.data[i].id == id) { 
				return &controls.data[i];
			}
		}
		return NULL;
	}
	control_t& add(control_t value) {
		return controls.add(value);
	}
	control_t* focussetcur(int x, int y) {
		 size_t i = 0;
		 for (control_t* ctrl = at(i); ctrl;) {
			 if (ctrl->inrange(x, y)) {
				 control_t::setfocus(ctrl);
				 return ctrl;
			 }
			 i++;
			 ctrl = at(i);
		 }
		 return NULL;
	}
	control_t* focusupdate(int x, int y) {
		control_t* focus = control_t::focus;
		control_t* ctrl1 = control_t::focusleave(x, y);
		BOOL cur_down = FALSE;
		if (ctrl1) {
			if (!ctrl1->singlesel) {
				cur_down = ctrl1->isdown;
				ctrl1->isdown = FALSE;
			}
			if (ctrl1->parent) {
				SendMessageA(ctrl1->parent, WM_USER_CONTROL_UP, ctrl1->id, WM_KILLFOCUS);
			}
		}
		if (focus == ctrl1) {
			control_t* ctrl2 = focussetcur(x, y);
			if (ctrl2) {
				if (!ctrl2->singlesel) {
					ctrl2->isdown = cur_down;
				}
				if (ctrl2->parent) {
					SendMessageA(ctrl2->parent, WM_USER_CONTROL_UP, ctrl2->id, WM_SETFOCUS);
				}
				return ctrl2;
			}
		}
		return NULL;
	}
};

static class WindowClass {
private:
	std::map<std::string, int> class_ref;
public:
	BOOL Register(std::string name,WndProcType pWndProc) {
		if (class_ref[name] > 0) {
			++class_ref[name];
			return TRUE;
		}
		WNDCLASSEXA wndclassc{};
		wndclassc.cbSize = sizeof(wndclassc);
		wndclassc.style = CS_HREDRAW | CS_VREDRAW;
		wndclassc.cbWndExtra = NULL;
		wndclassc.hInstance = dllself._module;
		wndclassc.lpfnWndProc = pWndProc;
		wndclassc.hIcon = NULL;
		wndclassc.hIconSm = NULL;
		wndclassc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
		wndclassc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndclassc.lpszClassName = name.data();
		BOOL retval= RegisterClassExA(&wndclassc) != NULL;
		if (retval) {
			class_ref[name]=1;
		}
		return retval;
	}
	BOOL UnRegister(std::string name) {
		if (class_ref[name] > 0) {
			--class_ref[name];
			if (class_ref[name]==0) {
				UnregisterClassA(name.data(), dllself._module);
			}
			return TRUE;
		}
		return TRUE;
	}
}WinClass;


int GetLogPixelsY();
float GetLogPixelsScale();
int ToFontSize(int height, bool scale_size=false);
int ToFontHeight(int size, bool scale_size=false);
BOOL GetClientRectExt(HWND hWnd, LPRECT lpRect);
BOOL MoveWindowExt(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
HFONT GetButtonFont();
HFONT CreateNewFont(LPCSTR fontname, int heigth, int Bold = FW_NORMAL, BOOL bItalic = FALSE,
	BOOL bUnderline = FALSE, BOOL bStrikeOut = FALSE);
void SetCtrlFont(HWND w, LPCSTR fontname, int width, int heigth, int Bold = FW_NORMAL, BOOL bItalic = FALSE,
	BOOL bUnderline = FALSE, BOOL bStrikeOut = FALSE);
int GetLogFont(HFONT hFont, LOGFONTA* lf);
int GetCtrlLogFont(HWND hwnd, LOGFONTA* lf);
HWND CreateButton(HWND parent, uint32_t id, DWORD dwStyle, LPCSTR name, LPCSTR fontname, int fontsize, int x, int y, int width, int heigth);

int GetEditTextLength(HWND hwnd);

std::string GetEditText(HWND hwnd, int maxlen,bool UseUTF8);
void SetEditText(HWND hwnd, std::string str);
BOOL GetCheck(HWND hwnd);
void SetCheck(HWND hwnd, BOOL enable);
void ShowHwnd(HWND hwnd, bool enable);
void ShowTopHwnd(HWND hwnd, bool enable);
void EnableHwnd(HWND hwnd, BOOL enable);
HWND GetChildCtrl(HWND hwnd,int id);
//CHOOSEFONT** pcf 放置临时结构地址指针(不安全)
BOOL ShowFontDialog(HWND hwnd, DWORD Flags, LOGFONTA* lf, LPCFHOOKPROC lpfnHook=NULL, CHOOSEFONTA** pcf = NULL, LPARAM userdata = NULL);
BOOL ShowColorDialog(HWND hwnd, DWORD Flags, COLORREF* lpc, LPCCHOOKPROC lpfnHook, LPARAM userdata=0);

void RemoveButtonDefStyle(HWND hwnd);

HWND CreateUpDnBuddy(HWND parent, HMENU hmenu, int x, int y, int width, int height);
HWND CreateUpDnCtl(HWND parent, HMENU hmenu, int min, int max);
HWND CreateGroupBox(HWND parent, HMENU hmenu, const char* name, int x, int y, int width, int height);
HWND CreateProgBar(HWND parent, HMENU hmenu, const char* name, int x, int y, int width, int height);

