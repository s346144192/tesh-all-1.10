#include <string.h>
#include <FuncList.h>
#include <tesh.h>

#include <Commctrl.h>
#pragma comment (lib, "comctl32.lib")


BaseMap<HWND, FuncListView*> FuncListView::classlist;

LRESULT CALLBACK FuncListView::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	FuncListView* flw = classlist.get(hwnd);
	if (!flw) {
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	switch (message)
	{
	case WM_SIZE:
		flw->Resize();
		break;
	case WM_NOTIFY:
	{
		SCNotification* notify = (SCNotification*)lParam;
		if (notify->nmhdr.hwndFrom == flw->_codebox._hwnd) {
			switch (notify->nmhdr.code)
			{
			case SCN_MARGINCLICK:
			{
				// 确定是页边点击事件  
				const int line_number = flw->_codebox.SendEditor(SCI_LINEFROMPOSITION, notify->position);
				const int lev = flw->_codebox.SendEditor(SCI_GETFOLDLEVEL, line_number, 0);
				//防止点击连接线折叠
				// (lev & SC_FOLDLEVELWHITEFLAG) 
				if (lev & SC_FOLDLEVELHEADERFLAG) {
					flw->_codebox.SendEditor(SCI_TOGGLEFOLD, line_number);
				}
				return TRUE;
			}
			case SCN_UPDATEUI:
			{
				if (notify->updated & SC_UPDATE_CONTENT || notify->updated & SC_UPDATE_SELECTION) {
					flw->_codebox.UpCurBrace();
				}
			}
			break;
			case SCN_HOTSPOTCLICK: {
				std::string str = flw->_codebox.GetHotspotWord(notify->position);
				if (str.length()) {
					int itemindex = flw->FindFunc(str.data());
					if (itemindex >= 0) {
						flw->LoadListFuncTextByIndex(itemindex);
					}
				}
			}
			break;
			default:
				break;
			}
		}
		return 0;
	}
	break;
	case WM_COMMAND:
	{
		WORD low = LOWORD(wParam);
		WORD high = HIWORD(wParam);
		// 分析菜单选择:
		flw->TriggerCtrlEvent((HWND)lParam, low, high);
		return TRUE;
		
	}
	break;
	case WM_CLOSE:
		ShowWindow(hwnd, SW_HIDE);
		return FALSE;
		break;
	case WM_DESTROY:
		//ShowWindow(hwnd, SW_HIDE);
		//return TRUE;
		break;
	}
	//DefWindowProc 极为重要，不然主窗口或其他窗口收不到消息。
	return DefWindowProc(hwnd, message, wParam, lParam);

}

char* stristr(const char* s1, const char* s2) {
	size_t len = strlen(s2);
	for (; *s1; ++s1) {
		if ((_strnicmp(s1, s2, len) == 0)) {
			return (char*)s1;
		}
	}
	return nullptr;
}


void FuncListView::ResetFuncList() {
	SendMessage(func_list, LB_RESETCONTENT, 0, 0);
}
void FuncListView::LoadFuncList(const char* match_str,jass_map& jass_text, bool gl_constant, bool reset) {
	SendMessage(func_list, WM_SETREDRAW, FALSE, 0L);
	
	if (reset) {
		ResetFuncList();
	}
	if (!match_str) {
		return;
	}
	const char* name = nullptr;
	const char* text = nullptr;
	size_t len = strlen(match_str);
	for (auto it = jass_text.begin(); it != jass_text.end();++it) {
		name =(*it).first.c_str();
		text= (*it).second.c_str();
		if (!gl_constant || strncmp(text,"constant",8)==0) {
			//if (!(*match_str) || (_strnicmp(name, match_str, len) == 0))
			if (!(*match_str) || (stristr(name, match_str) != 0)) {
				SendMessage(func_list, LB_ADDSTRING, 0, (LPARAM)name);
			}
		}
	}

	SendMessage(func_list, WM_SETREDRAW, TRUE, 0L);
}
void FuncListView::LoadFuncList() {
	ResetFuncList();
	char match_str[260] = { 0 };
	SendMessage(_textbox_search_input, WM_GETTEXT, (WPARAM)sizeof(match_str), (LPARAM)match_str);
	BOOL is_selected_function= SendMessage(_select_functions, BM_GETCHECK, 0, 0);
	if (SendMessage(_checkbox_natives, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		if (is_selected_function) {
			LoadFuncList((const char*)match_str,cj_text, false,false);
		}
		else {
			LoadFuncList((const char*)match_str, cj_globals_text, false, false);
		}
	}
	if (SendMessage(_checkbox_BJs, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		if (is_selected_function) {
			LoadFuncList((const char*)match_str, bj_text, false, false);
		}
		else {
			LoadFuncList((const char*)match_str, bj_globals_text, false, false);
		}
	}
	if (SendMessage(_checkbox_custom, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		if (is_selected_function) {
			LoadFuncList((const char*)match_str, custom_function_text, false, false);
		}
		else {
			LoadFuncList((const char*)match_str, custom_globals_text, true, false);
		}
	}
	if (*match_str) {
		int index = SendMessage(func_list, LB_FINDSTRING, -1, (LPARAM)match_str);
		index = max(0, index);
		if (index >= 0) {
			SendMessage(func_list, LB_SETTOPINDEX, index, 0L);
			SetCurSel(index);
		}
	}
}
FuncListView::FuncListView() {
	_main = NULL;
}
FuncListView::~FuncListView() {

}

void FuncListView::splitter_up(HWND parent, HWND hwnd) {
	FuncListView* flw = classlist.get(parent);
	splitter_t* splitter = GetSplitter(hwnd);
	if (!flw || !splitter) {
		return;
	}
	if (&(flw->splitter) != splitter) {
		return;
	}
	if (!splitter->uppoint()) {
		return;
	}
	flw->UpPoint();
}
std::string FuncListView::get_func_codetext(const char* name) {
	if (!name) {
		return "";
	}
	static jass_map* jass_text_list[] = { &cj_text, &bj_text, &custom_function_text
			,&cj_globals_text, &bj_globals_text, &custom_globals_text };
	static const int max = sizeof(jass_text_list) / sizeof(jass_map*);
	std::string text;
	for (int i = 0; i < max; i++) {
		text=jass_text_list[i]->get(name);
		if (text.length() > 0) {
			if (text.back() != '\n') {
				text.push_back('\n');
			}
			return text;
		}
	}
	return "";

}
void FuncListView::SetFuncText(const char* text) {
	if (text ) {
		_codebox.SetReadonly(FALSE);
		_codebox.SendEditor(SCI_SETTEXT, (uptr_t)0, (uptr_t)text);
		_codebox.SetReadonly(TRUE);
	}
}
void FuncListView::LoadListFuncTextByIndex(int i) {
	if (i >= 0) {
		char name[360] = { 0 };
		std::string text = "";
		int len = SendMessage(func_list, LB_GETTEXT, i, (LPARAM)name);
		if (len > 0) {
			text = get_func_codetext(name);
		}
		SetFuncText(text.c_str());
	}
	else {
		SetFuncText("");
	}
}
void FuncListView::LoadCurSelFuncText() {
	int select = SendMessage(func_list, LB_GETCURSEL, 0, 0);
	LoadListFuncTextByIndex(select);
}

int FuncListView::FindFunc(const char* name) {
	return SendMessage(func_list, LB_FINDSTRING, (WPARAM)-1, (LPARAM)name );
}
void FuncListView::SetCurSel(int i) {
	SendMessage(func_list, LB_SETCURSEL, i, 0);
	LoadCurSelFuncText();
}
void FuncListView::SetFindCurSel(const char* name) {
	int i = FindFunc(name);
	if (i>=0) {
		SetCurSel(i);
	}
}
void FuncListView::TriggerCtrlEvent(HWND hwnd, WORD ctrl, WORD eventid) {
	if (ctrl== IDR_FUNCLIST_LIST) {
		if (eventid == LBN_SELCHANGE || eventid == LBN_SELCANCEL) {
			LoadCurSelFuncText();
		}
		return;
	}
	if (ctrl == IDR_FUNCLIST_SEARCH_INPUT) {
		if (eventid == EN_CHANGE) {
			LoadFuncList();
		}
		return;
	}
	if (eventid == BN_CLICKED)
		if (hwnd == _checkbox_natives) {
			LoadFuncList();
		}
		else if (hwnd == _checkbox_BJs) {
			LoadFuncList();
		}
		else if (hwnd == _checkbox_custom) {
			LoadFuncList();
		}
		else if (hwnd == _select_functions) {
			BOOL state = SendMessage(hwnd, BM_GETCHECK, 0, 0);
			if (state == BST_CHECKED) {
				EnableWindow(_checkbox_natives, TRUE);
				EnableWindow(_checkbox_BJs, TRUE);
				EnableWindow(_checkbox_custom, TRUE);
			}

			LoadFuncList();
			
		}
		else if (hwnd == _select_constants) {
			BOOL state = SendMessage(hwnd, BM_GETCHECK, 0, 0);
			if (state == BST_CHECKED) {
				EnableWindow(_checkbox_natives, FALSE);
				EnableWindow(_checkbox_BJs, FALSE);
				EnableWindow(_checkbox_custom, FALSE);
			}
			
			LoadFuncList();
			
		}
		else if (hwnd == btn_up_down_show) {
			btn_up_down_winstate = btn_up_down_winstate ^ SW_SHOW;
			ShowWindow(_checkbox_natives, btn_up_down_winstate);
			ShowWindow(_checkbox_BJs, btn_up_down_winstate);
			ShowWindow(_checkbox_custom, btn_up_down_winstate);
			ShowWindow(_select_functions, btn_up_down_winstate);
			ShowWindow(_select_constants, btn_up_down_winstate);
			POINT pos;
			RECT rt;
			GetWindowRect(hwnd, &rt);
			pos = { rt.left,rt.top };
			ScreenToClient(_main, &pos);
			GetClientRect(func_list, &rt);
			if (btn_up_down_winstate == SW_SHOW) {
				SetWindowTextA(btn_up_down_show, "∧");
				pos.y = pos.y + 64;
				rt.bottom = rt.bottom - 64;
			}
			else {
				SetWindowTextA(btn_up_down_show, "∨");
				pos.y = pos.y - 64;
				rt.bottom = rt.bottom + 64;
			}
			SetWindowPos(hwnd, NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE);
			if (GetWindowLongA(func_list, GWL_STYLE)& WS_VSCROLL) {
				rt.right += GetSystemMetrics(SM_CXVSCROLL);
			}
			if (GetWindowLongA(func_list, GWL_STYLE)& WS_HSCROLL) {
				rt.bottom += GetSystemMetrics(SM_CXHSCROLL);
			}
			SetWindowPos(func_list, NULL, pos.x, pos.y + 16, rt.right + 4, rt.bottom + 4, SWP_DRAWFRAME);
			RedrawWindow(func_list, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
		}
}
BOOL FuncListView::Show(int ishow ) {
	static BOOL one_setpos = FALSE;
	//初始化上级窗口位置不正确,重新设置一次。
	if (!one_setpos && _parent!=NULL && ishow==SW_SHOW) {
		one_setpos = TRUE;
		RECT rect;
		GetWindowRect(_parent, &rect);
		SetWindowPos(_main, NULL, rect.left + ((rect.right - rect.left) / 2 - 375), rect.top + ((rect.bottom - rect.top) / 2 - 225), 0, 0, SWP_NOSIZE);
	}
	return ShowWindow(_main, ishow);
}
void FuncListView::UpPoint() {
	POINT pos;
	RECT rect;
	RECT rect2;
	int x = 0;
	int y = 0;
	GetClientRect(_main, &rect);
	GetClientRect(splitter._hwnd, &rect2);
	pos = { rect2.left,rect2.top };
	ClientToScreen(splitter._hwnd, &pos);
	ScreenToClient(_main, &pos);
	x = pos.x + 3;
	GetClientRect(_codebox._hwnd, &rect2);
	SetWindowPos(_codebox._hwnd, NULL, 0, 0, x, rect2.bottom, SWP_NOMOVE);

	MoveWindowExt(_textbox_search_input, x, -1, rect.right - x, -1, FALSE);

	MoveWindowExt(_checkbox_natives, x, -1, -1, -1, FALSE);
	MoveWindowExt(_checkbox_BJs, x, -1, -1, -1, FALSE);
	MoveWindowExt(_checkbox_custom, x, -1, -1, -1, FALSE);

	MoveWindowExt(_select_functions, x + 100, -1, -1, -1, FALSE);
	MoveWindowExt(_select_constants, x + 100, -1, -1, -1, FALSE);

	MoveWindowExt(btn_up_down_show, x, -1, rect.right - x, -1, FALSE);

	GetClientRectExt(btn_up_down_show, &rect2);

	MoveWindowExt(func_list, x, -1, rect.right - x, rect.bottom - rect2.bottom - 0, FALSE);
	
	MoveWindow(_codebox._hwnd, 0, 0, x - 3, rect.bottom, FALSE);

	//InvalidateRect(_main, NULL, FALSE);
	//UpdateWindow(_main);
	RedrawWindow(_main, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}
void FuncListView::Resize() {
	RECT rect;
	RECT rect2;
	int x = 0;
	int y = 0;
	GetClientRect(_main, &rect);
	GetClientRectExt(_textbox_search_input, &rect2);
	x = rect.right - (rect2.right - rect2.left);

	MoveWindow(splitter._hwnd, x - 3, 0, 3, rect.bottom, FALSE);

	MoveWindowExt(_textbox_search_input, x, -1, -1, -1, FALSE);

	MoveWindowExt(_checkbox_natives, x, -1, -1, -1, FALSE);
	MoveWindowExt(_checkbox_BJs, x, -1, -1, -1, FALSE);
	MoveWindowExt(_checkbox_custom, x, -1, -1, -1, FALSE);

	MoveWindowExt(_select_functions, x + 100, -1, -1, -1, FALSE);
	MoveWindowExt(_select_constants, x + 100, -1, -1, -1, FALSE);

	MoveWindowExt(btn_up_down_show, x, -1, -1, -1, FALSE);

	GetClientRectExt(btn_up_down_show, &rect2);

	MoveWindowExt(func_list, x, -1, -1, rect.bottom - rect2.bottom - 0, FALSE);
	
	MoveWindow(_codebox._hwnd, 0, 0, x - 3, rect.bottom, FALSE);

	RedrawWindow(_main, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}
BOOL FuncListView::Init(HWND parent) {
	
	if (_main)
	{
		return TRUE;
	}
	InitCommonControls();

	int x = 0;
	int y = 0;
	int width = 225;
	int height = 0;
	int scrWidth = GetSystemMetrics(SM_CXSCREEN);
	int scrHeight = GetSystemMetrics(SM_CYSCREEN);
	RECT rect;
	_parent = parent;
	WinClass.Register("FuncListWin", WndProc);
	_main = CreateWindowExA(WS_EX_TOOLWINDOW, "FuncListWin", "函数列表", WS_SIZEBOX | WS_SYSMENU
		, max(0,(scrWidth / 2 - 450)), max(0, (scrHeight / 2 - 225)), 900, 450, parent, (HMENU)0, nullptr, nullptr);

	//ShowWindow(_main, SW_HIDE);
	GetClientRect(_main, &rect);

	x = rect.right - width;
	y = 2;
	//IDC_SIZEWE
	splitter.init(_main, { x - 3, 0, 3, rect.bottom }, MAKEINTRESOURCEA(32644), IDR_FUNCLIST_SPLITTER,
		splitter_t::horizontal, { 16,16,16,16 }, splitter_up);

	_textbox_search_input = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit"
		, NULL, WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | WS_VISIBLE
		, x, y, width, 24, _main, (HMENU)IDR_FUNCLIST_SEARCH_INPUT, nullptr, nullptr);
	SendMessageA(_textbox_search_input, EM_LIMITTEXT, 150, 0);
	y = y + 26;
	_checkbox_natives = CreateWindowA("Button", "natives"
		, WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, x, y, 100, 18, _main, (HMENU)IDR_FUNCLIST_CB_NATIVES, nullptr, nullptr);

	y = y + 22;
	_checkbox_BJs = CreateWindowA("Button", "BJs"
		, WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, x, y, 100, 18, _main, (HMENU)IDR_FUNCLIST_CB_BJS, nullptr, nullptr);

	y = y + 22;
	_checkbox_custom = CreateWindowA("Button", "custom"
		, WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_LEFT | WS_VISIBLE
		, x, y, 100, 18, _main, (HMENU)IDR_FUNCLIST_CB_CUSTOM, nullptr, nullptr);


	y = y - (22 * 3 - 12) + 22;
	_select_functions = CreateWindowA("Button", "functions"
		, WS_CHILD | BS_AUTORADIOBUTTON | BS_LEFT | WS_VISIBLE
		, x + 100, y, 150, 18, _main, (HMENU)IDR_FUNCLIST_SEL_FUNCTIONS, nullptr, nullptr);

	y = y + 22;
	_select_constants = CreateWindowA("Button", "constants"
		, WS_CHILD | BS_AUTORADIOBUTTON | BS_LEFT | WS_VISIBLE
		, x + 100, y, 150, 18, _main, (HMENU)IDR_FUNCLIST_SEL_CONSTANTS, nullptr, nullptr);

	y = y + 30;
	btn_up_down_show = CreateWindowA("Button", "∧"
		, WS_CHILD | ES_CENTER | BS_PUSHBUTTON | WS_VISIBLE
		, x, y, width, 18, _main, (HMENU)IDR_FUNCLIST_BTN_UP_DOWN, nullptr, nullptr);

	y = y + 18;
	height = rect.bottom - y;

	func_list = CreateWindowExA(WS_EX_CLIENTEDGE , "ListBox", NULL
		, WS_CHILD | WS_VISIBLE  | LBS_NOINTEGRALHEIGHT | LBS_SORT | LBS_NOTIFY | WS_VSCROLL
		, x, y, width, height, _main, (HMENU)IDR_FUNCLIST_LIST, nullptr, nullptr);
	
	_codebox.InitEditBox(WS_EX_CLIENTEDGE
		, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE
		, 0, 0, x - 3, rect.bottom, _main, (HMENU)IDR_FUNCLIST_CODEBOX, nullptr);

	//SetCtrlFont(_main, "Calibri", 0, 20, FW_NORMAL);
	SetCtrlFont(_textbox_search_input, "Tahoma", 0, 17);
	SetCtrlFont(_checkbox_natives, "Tahoma", 0, 17);
	SetCtrlFont(_checkbox_BJs, "Tahoma", 0, 17);
	SetCtrlFont(_checkbox_custom, "Tahoma", 0, 17);
	SetCtrlFont(_select_functions, "Tahoma", 0, 17);
	SetCtrlFont(_select_constants, "Tahoma", 0, 17);
	//if (win_ver.num <= 7) {
	//	SetCtrlFont(btn_up_down_show, "Calibri", 6, 11, FW_BOLD);
	//}
	//else {
		SetCtrlFont(btn_up_down_show, "Tahoma", 11, 15, FW_MEDIUM);
	//}
	
	SetCtrlFont(func_list, "Tahoma", 0, 16, FW_NORMAL);

	btn_up_down_winstate = SW_SHOW;

	classlist.add(_main,this);
	//默认选择
	SendMessage(_checkbox_natives, BM_SETCHECK, BST_CHECKED, 0);
	SendMessage(_checkbox_BJs, BM_SETCHECK, BST_CHECKED, 0);
	SendMessage(_checkbox_custom, BM_SETCHECK, BST_CHECKED, 0);
	SendMessage(_select_functions, BM_SETCHECK, BST_CHECKED, 0);

	_codebox.SetReadonly(TRUE);
	_codebox.SetLexer(SCLEX_JASS); //语法解析

	(_codebox.IsEnableUTF8) ? _codebox.SetCodePage(SC_CP_UTF8) : _codebox.SetCodePage(SC_CP_GBK);

	//默认文本
	_codebox.SetDefaultStyles("Courier New", 10, RGB(0x00, 0x00, 0x00), RGB(0xff, 0xff, 0xff));

	_codebox.EnableFold(1, 14, true, true);
	_codebox.SetMarkerFore(RGB(0, 255, 255));
	_codebox.SetMarkerback1(RGB(0, 0, 255));
	_codebox.SetMarkerback2(RGB(0, 255, 255));
	// 显示行号
	_codebox.EnableLineNumber(0, 36);
	//设置TAB 宽度
	_codebox.SetTabWidth(4);
	//选中的颜色
	_codebox.SetSelectStyle(TRUE, RGB(255, 255, 255), RGB(51, 153, 255), SC_ALPHA_NOALPHA);

	for (int i = SCE_JASS_DEFAULT; i <= SCE_JASS_NMAX; i++) {
		_codebox.SetFont(i, "Courier New", true);
	}
	_codebox.SetFont(STYLE_LINENUMBER, "Courier New", true);
	_codebox.SetBraceStyle("Courier New", true, RGB(0, 0, 0), RGB(255, 0, 0), TRUE);

	_codebox.SetFontBold(SCE_JASS_WORD, TRUE);
	_codebox.SetFontBold(SCE_JASS_WORD2, TRUE);
	_codebox.SetForeColor(SCE_JASS_WORD3, RGB(0, 0, 170));//SetFontBold(SCE_JASS_WORD3, TRUE); 
	_codebox.SetFontBold(SCE_JASS_WORD4, TRUE); _codebox.SetForeColor(SCE_JASS_WORD4, RGB(0, 85, 170));

	_codebox.SetForeColor(SCE_JASS_WORD5, RGB(0, 58, 213));
	_codebox.SetForeColor(SCE_JASS_WORD6, RGB(173, 58, 128));
	_codebox.SetForeColor(SCE_JASS_WORD7, RGB(255, 0, 58));

	/*SetStyleHotspot(SCE_JASS_WORD5, TRUE);
	SetStyleHotspot(SCE_JASS_WORD6, TRUE);*/

	_codebox.SetFontItalic(SCE_JASS_COMMENT, TRUE); _codebox.SetForeColor(SCE_JASS_COMMENT, RGB(0, 136, 0));
	_codebox.SetFontItalic(SCE_JASS_COMMENTLINE, TRUE); _codebox.SetForeColor(SCE_JASS_COMMENTLINE, RGB(0, 136, 0));
	_codebox.SetFontItalic(SCE_JASS_COMMENTFLAG, TRUE); _codebox.SetForeColor(SCE_JASS_COMMENTFLAG, RGB(0, 136, 0));
	_codebox.SetForeColor(SCE_JASS_NUMBER, RGB(0, 0, 170));

	_codebox.SetFontItalic(SCE_JASS_STRING, TRUE); _codebox.SetForeColor(SCE_JASS_STRING, RGB(0, 58, 213));
	_codebox.SetFontBold(SCE_JASS_CHARACTER, TRUE); _codebox.SetFontUnderline(SCE_JASS_CHARACTER, TRUE); //SetForeColor(SCE_JASS_CHARACTER, RGB(173, 58, 128));
	_codebox.SetFontBold(SCE_JASS_PREPROCESSOR, TRUE);// SetForeColor(SCE_JASS_PREPROCESSOR, RGB(173, 58, 128));
	_codebox.SetFontBold(SCE_JASS_OPERATOR, TRUE); _codebox.SetForeColor(SCE_JASS_OPERATOR, RGB(170, 85, 0));
	//SetForeColor(SCE_JASS_IDENTIFIER, RGB(0, 0, 0));
	_codebox.SetForeColor(SCE_JASS_RUNTEXTMACRO, RGB(153, 153, 153));

	_codebox.SetForeColor(SCE_LUA_COMMENT, RGB(0, 136, 0));
	_codebox.SetForeColor(SCE_LUA_COMMENTLINE, RGB(0, 136, 0));
	_codebox.SetForeColor(SCE_LUA_COMMENTDOC, RGB(0, 136, 0));
	_codebox.SetForeColor(SCE_LUA_NUMBER, RGB(0, 0, 170));
	_codebox.SetFontBold(SCE_LUA_WORD, TRUE); //SetForeColor(SCE_LUA_WORD, RGB(197,134, 161));
	_codebox.SetForeColor(SCE_LUA_STRING, RGB(0, 58, 213));
	//SetFontBold(SCE_LUA_CHARACTER, TRUE);// SetForeColor(SCE_LUA_CHARACTER, RGB(0, 58, 213));
	_codebox.SetForeColor(SCE_LUA_LITERALSTRING, RGB(0, 58, 213));
	//SetForeColor(SCE_LUA_PREPROCESSOR, RGB(0, 58, 213));
	_codebox.SetFontBold(SCE_LUA_OPERATOR, TRUE); _codebox.SetForeColor(SCE_LUA_OPERATOR, RGB(170, 85, 0));
	_codebox.SetFontBold(SCE_LUA_IMPORT_FALG, TRUE); _codebox.SetForeColor(SCE_LUA_IMPORT_FALG, RGB(170, 85, 0));//[[]] style = SCE_LUA_OPERATOR
	//SetForeColor(SCE_LUA_IDENTIFIER, RGB(255, 153, 0));
	//SetForeColor(SCE_LUA_LABEL, RGB(0, 58, 213));
	//SetForeColor(SCE_LUA_IMPORT_FALG, RGB(0, 58, 213));

	_codebox.SetForeColor(SCE_LUA_WORD5, RGB(0, 58, 213));
	_codebox.SetForeColor(SCE_LUA_WORD6, RGB(173, 58, 128));
	_codebox.SetForeColor(SCE_LUA_WORD7, RGB(255, 0, 58));

	_codebox.SetKeywords(1, LuaKeywords);
	_codebox.SetKeywords(0, JassKeywords);
	_codebox.SetKeywords(2, JassKeywords2);
	_codebox.SetKeywords(3, JassKeywords3);
	_codebox.SetKeywords(4, JassTypes);

	UpdateWindow(_main);
	return TRUE;
}
