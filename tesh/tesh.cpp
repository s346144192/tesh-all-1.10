#include <io.h>
#include <windows.h>
#include <base/hook/fp_call.h>
#include <hookwe.h>
#include <base/encode/convert_utf_8.h>
#include <dll_info.h>
#include <text/simple_analys.h>
#include <tesh.h>
#include <TsfApp.h>
#include <CommCtrl.h>
#include <DuiLibLoader.h>

extern uintptr_t pTrueCreateWindowExA;
extern uintptr_t pTrueSetMenu;
extern uintptr_t pTrueShowWindow;
extern uintptr_t pTrueSetWindowTextA;
extern uintptr_t pTrueDestroyWindow;
extern uintptr_t pTrueMessageBoxA;
extern uintptr_t pTruePeekMessageA;

TESH Tesh;
TESH* TeshModule= NULL;
ini::config_t TESH::config, TESH::styles;

jass_map cj_text, bj_text, custom_function_text;
jass_map cj_globals_text, bj_globals_text, custom_globals_text;

//jass_map TESH::cj_text, TESH::bj_text, TESH::custom_function_text;
//jass_map TESH::cj_globals_text, TESH::bj_globals_text, TESH::custom_globals_text;
HWND TESH::WorldEditorMain = NULL;
HWND TESH::WeToolbar = NULL;
HWND TESH::TriggerEditor=NULL;
//HWND TESH::TE_TrigTreeView = NULL;
HMENU TESH::WE_Menu = NULL;
HMENU TESH::TeshMenu = NULL;
HWND TESH::Edit = NULL;
WNDPROC TESH::pWE_WndProc = NULL;
WNDPROC TESH::pTE_WndProc = NULL;
WNDPROC TESH::pEdit_WndProc = NULL;
BOOL  TESH::ShowAnyPopCtrlAllSkipIncIme=FALSE;

void FixIme() {
	if (win_ver.num < 8)
	{ 
		return;
	}
	TsfApp ime_app;
	ime_app.SetupSinks();
	size_t i = 1;
	size_t count = ime_app.GetIMEMaxCount();
	wchar_t* ime_name = ime_app.GetDefaultIMEName();
	while (i< count && ( wcscmp(ime_name,L"微软五笔")==0 || wcscmp(ime_name, L"微软拼音")==0) ) {
		ime_name = ime_app.NextIMEName(TRUE);
		i++;
	}
	ime_app.ReleaseSinks();
}

LRESULT CALLBACK TESH::DetourWeWindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	//printf("message:%03x\n", message);
	return base::std_call<LRESULT>(pWE_WndProc, windowHandle, message, wParam, lParam);
}

HWND WINAPI TESH::DetourWeCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	HWND result = base::std_call<HWND>(pTrueCreateWindowExA,
		dwExStyle,
		lpClassName,
		lpWindowName,
		dwStyle,
		X,
		Y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		hInstance,
		lpParam
		);
	//if (hWndParent == TriggerEditor) {
	//	if ((int)lpClassName > 0xFFFF) {
	//		if ((int)hMenu == 0xC && strcmp(lpClassName, "SysTreeNodeView32") == 0) {
	//			TeTrigTreeView = result;
	//			printf("TrigTreeView:%x\n", TeTrigTreeView);
	//		}
	//	}
	//	else {
	//		if ((int)hMenu == 0xC && (int)lpClassName == ICC_TREEVIEW_CLASSES) {
	//			TeTrigTreeView = result;
	//			printf("TrigTreeView:%x\n", TeTrigTreeView);
	//		}
	//	}
	//}
	//if (strcmp("Warcraft III", lpClassName) == 0)
	//{
	//	WorldEditorMain = result;
	//	//printf("WorldEditorMain:%x\n", WorldEditorMain);
	//}
	//else if (WorldEditorMain!=NULL && hWndParent== WorldEditorMain) {
	//	if (lpClassName) {
	//		if (strcmp(lpClassName, "ToolbarWindow32") == 0 && (int)hMenu == 10) {
	//			WeToolbar = result;
	//		}
	//	}
	//}
	return result;
}
BOOL WINAPI TESH::DetourWeSetMenu(HWND hWnd, HMENU hMenu)
{
	if (hWnd == WorldEditorMain && WE_Menu == NULL)
	{
		WE_Menu = hMenu;
		pWE_WndProc = (decltype(pWE_WndProc))GetWindowLongPtrA(WorldEditorMain, GWL_WNDPROC);
		SetWindowLongA(WorldEditorMain, GWL_WNDPROC, reinterpret_cast<LONG>(DetourWeWindowProc));
	}

	return base::std_call<BOOL>(pTrueSetMenu, hWnd, hMenu);
}

BOOL WINAPI TESH::DetourWeShowWindow(HWND hwnd, int nCmdShow) {
	//BOOL result = 0;
	if (TeshModule) {
		if (ShowAnyPopCtrlAllSkipIncIme && nCmdShow == SW_SHOW) {
			FixIme();
		}
		TeshModule->InitTriggerEditor(hwnd, nCmdShow);
		if (TeshModule->InitEditText(hwnd, nCmdShow)) {
			if (hwnd == Edit) {
				if (nCmdShow == SW_SHOW) {
					return ShowWindow(TeshModule->CodeEditBox._main, nCmdShow);
				}
				else if (nCmdShow == SW_HIDE) {
					//ShowWindow(TeshModule->_Options._main, nCmdShow);
					//ShowWindow(TeshModule->FuncListWin._main, nCmdShow);
					ShowWindow(TeshModule->CodeEditBox._findtext._main, nCmdShow);
				}
				ShowWindow(TeshModule->CodeEditBox._main, nCmdShow);
			}
		}
	}
	return ShowWindow(hwnd, nCmdShow);
}

BOOL WINAPI TESH::DetourWeSetWindowTextA(HWND hwnd, LPCSTR lpString) {
	if (TeshModule) {
		if (Edit == hwnd) {
			//log_put("Edit", "SetWindowTextA %x ", "", result);
			//printf("设置文本\n");
			 SetWindowTextA(hwnd, lpString);
			return TeshModule->CodeEditBox.SetEditPutText(lpString);
		}
	}
	return SetWindowTextA(hwnd, lpString);
}

BOOL WINAPI TESH::DetourWeDestroyWindow(HWND hwnd) {
	if (hwnd && Edit==hwnd && TeshModule) {
		TeshModule->SaveEditText();
		DestroyWindow(TeshModule->CodeEditBox._main);
		//ShowWindow(TeshModule->FuncListWin._main, SW_HIDE);
	}
	return DestroyWindow(hwnd);
}

int WINAPI TESH::DetourWeMessageBoxA(HWND hwnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
	if (Tesh.next_convert_to_code_dialog){
		Tesh.next_convert_to_code_dialog = FALSE;
		if (!Tesh._Options.ShowWarning 
			&& Tesh.TriggerEditor == hwnd
			&& (!strcmp(lpCaption, "Warning") || !strcmp(lpCaption, "警告"))) {
			return 1;
		}
	}
	return MessageBoxA(hwnd, lpText, lpCaption, uType);
}

BOOL WINAPI TESH::DetourWePeekMessageA(LPMSG lpMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
	BOOL result = PeekMessageA(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	if (!Tesh.TriggerEditor) {
		return result;
	}
	switch (lpMsg->message)
	{
	case WM_KEYDOWN:
	{
		static CodeEditView* CodeEdit = &(Tesh.CodeEditBox);
		if (lpMsg->wParam == VK_CONTROL) {
			if (CodeEdit->_hwnd == lpMsg->hwnd ) {
				CodeEdit->EnableHotspot(TRUE);
			}
			if (TeshModule->FuncListWin._codebox._hwnd == lpMsg->hwnd) {
				TeshModule->FuncListWin._codebox.EnableHotspot(TRUE);
			}
			return result;
		}
		if (lpMsg->wParam == VK_F3) {
			if (lpMsg->hwnd == CodeEdit->_main
				|| GetParent(lpMsg->hwnd) == CodeEdit->_main
				|| lpMsg->hwnd == CodeEdit->_hwnd
				|| lpMsg->hwnd == CodeEdit->_findtext._main
				|| GetParent(lpMsg->hwnd) == CodeEdit->_findtext._main) {
				if ((CodeEdit->_findtext.isshow()) ) {
					if (GetKeyState(VK_SHIFT) < 0) {
						CodeEdit->search_prev();
					}
					else {
						CodeEdit->search_next();
					}
				}
				//=====================屏蔽F3============================
				if (GetMessageA(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax)) {
					DispatchMessageA(lpMsg);
				}
				return PeekMessageA(lpMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
				//=======================================================
			}
		}
		if (CodeEdit->_hwnd == lpMsg->hwnd) {
			if (GetKeyState(VK_CONTROL) < 0) {
				switch (lpMsg->wParam)
				{
				case 'A':
					CodeEdit->SendEditor(SCI_SELECTALL);
					break;
				case 'C':
					CodeEdit->SendEditor(SCI_COPY);
					break;
				case 'F':
					//如果没打开则折叠,如果已打开保持。
					CodeEdit->CallCtrlF();
					break;
				case 'H':
					CodeEdit->CallCtrlH();
					break;
				case 'V':
					CodeEdit->SendEditor(SCI_PASTE);
					break;
				case 'X':
					CodeEdit->SendEditor(SCI_CUT);
					break;
				case 'Y':
					CodeEdit->SendEditor(SCI_REDO);
					break;
				case 'Z':
					CodeEdit->SendEditor(SCI_UNDO);
					break;
				}
			}
		}
		else if ( CodeEdit->_findtext._findput == lpMsg->hwnd
			 || CodeEdit->_findtext._replaceput == lpMsg->hwnd){
			if (GetKeyState(VK_CONTROL) < 0) {
				switch (lpMsg->wParam)
				{
				case 'A':
					SendMessageA(lpMsg->hwnd, EM_SETSEL, 0, -1);
					break;
				case 'C':
					SendMessageA(lpMsg->hwnd, WM_COPY, 0, 0);
					break;
				case 'V':
					SendMessageA(lpMsg->hwnd, WM_PASTE, 0, 0);
					break;
				case 'X':
					SendMessageA(lpMsg->hwnd, WM_CUT, 0, 0);
					break;
				case 'Z':
					SendMessageA(lpMsg->hwnd, WM_UNDO, 0, 0);
					break;
				}
			}
		}
	}
	break;
	case WM_KEYUP:
		if (lpMsg->wParam == VK_CONTROL) {
			if (TeshModule->CodeEditBox._hwnd != NULL) {
				TeshModule->CodeEditBox.EnableHotspot(FALSE);
			}
			if (TeshModule->FuncListWin._codebox._hwnd != NULL) {
				TeshModule->FuncListWin._codebox.EnableHotspot(FALSE);
			}
			return result;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if (lpMsg->hwnd != Tesh.CodeEditBox._hwnd) {
			if (Tesh.CodeEditBox._edit_state != 0
				//&& GetParent(lpMsg->hwnd) == Tesh.TriggerEditor
				&& GetDlgCtrlID(lpMsg->hwnd) == 11
				) {
				Tesh.SaveEditText();
			}
		}
		break;
	default:
		break;
	}
	return result;
}


LRESULT CALLBACK TESH::CodeEditFocus(HWND hwnd, BOOL enable) {
	if (TeshModule != NULL) {
		if (TeshModule->CodeEditBox._hwnd== hwnd 
			&& TeshModule->CodeEditBox._edit_state!=0
			&& !enable  /* 离开焦点时保存*/) {
			TeshModule->SaveEditText();
		}
	}
	return TRUE;
}

LRESULT CALLBACK TESH::MenuProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (LOWORD(wParam))
	{
	case 10010:
		Tesh._Options.Show(true);
		return TRUE;
	case 10020:Tesh.FuncListWin.Show(SW_SHOW);
		return TRUE;
	case 10030:
		Tesh._TeshAbout.Show(true);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}
LRESULT CALLBACK TESH::TE_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	//LRESULT result = 0;
	if (!TeshModule) {
		return CallWindowProcA(pTE_WndProc, hwnd, message, wParam, lParam);
	}
	switch (message)
	{
	//case WM_DESTROY:
		//case WM_NCMOUSEMOVE:
		//case WM_MOUSEACTIVATE:
	case WM_MOVE:
	{
		if (Tesh.CodeEditBox._hwnd != NULL) {
			Tesh.CodeEditBox.AutoCompleteUpdate();
			Tesh.CodeEditBox.SetCallTipUpdate();
		}
	}
	break;
	case WM_KILLFOCUS:
	case WM_CLOSE:
		if (TeshModule->CodeEditBox._edit_state != 0) {
			TeshModule->SaveEditText();
			//重新发送消息。(注意防止无限循环!)
			return SendMessageA(hwnd, message, wParam, lParam);
		}
		break;
	case WM_NOTIFY:
		//一般在列表框会发送此消息。(过于频繁)
		////触发器节点列表
		//if (wParam==13 && TE_TrigTreeView) {
		//	LPNMTREEVIEWA pnmtv = (LPNMTREEVIEWA)lParam;
		//	if (pnmtv) {
		//		if (pnmtv->action== TVN_SELCHANGING) {
		//			printf("选择即将更改\n");
		//		}
		//	}
		//}
		break;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				//屏蔽消息：(快捷键更改id仍然一致,HIWORD(wParam)固定值为1)
				//仅用于快捷键
				case 0x0:   //创建新地图
				case 0x1:   //打开地图
				case 0x2:   //关闭地图
				case 0x4:   //保存地图
				case 0x6:   //计算阴影并保存地图
				case 0xE:   //测试地图
				case 0x10:   //关闭触发器模块
				case 0x0a:   //导入触发器
				case 0x0b:   //导出触发器
					//手动保存
					if (HIWORD(wParam) == 1 
						&& IsWindowVisible(TeshModule->CodeEditBox._main)
						&& TeshModule->CodeEditBox._edit_state != 0){
						TeshModule->SaveEditText();
						return SendMessageA(hwnd, message, wParam, lParam);
					}
					break;
				case 0x1e:   //测试
					if (GetDlgCtrlID((HWND)lParam) == 10)
					{
						if (IsWindowVisible(TeshModule->CodeEditBox._main)
							&& TeshModule->CodeEditBox._edit_state != 0) {
							TeshModule->SaveEditText();
							return SendMessageA(hwnd, message, wParam, lParam);
						}
					}
					break;
				case 0x100: //复制
				case 0x106: //粘贴
					if (HIWORD(wParam) == 1 && GetFocus() == TeshModule->CodeEditBox._hwnd) {
						return FALSE;
					}
					break;
				case 0x114: //转换自定义代码
					Tesh.next_convert_to_code_dialog=TRUE;
					break;
				case 0x200: //新建类别
				case 0x202: //新建注释
					if (HIWORD(wParam) == 1 && GetFocus() == TeshModule->CodeEditBox._hwnd) {
						return FALSE;
					}
					break;
				default:
					//printf("id:%d\n", LOWORD(wParam));
					MenuProc(hwnd, message, wParam, lParam);
				break;
			}
		}
		break;
	default:
		//printf("message:%d,wParam:%d,lParam:%d\n", message, wParam, lParam);
		break;
	}
	return CallWindowProcA(pTE_WndProc, hwnd, message, wParam, lParam);
}
LRESULT CALLBACK TESH::Edit_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	//log_put("Edit", "msg:0x%x , wParam:%d, lParam:%d", message, wParam, lParam);
	LRESULT result = CallWindowProcA(pEdit_WndProc, hwnd, message, wParam, lParam);
	switch (message)
	{
	case WM_MOVE:
	{
		Tesh.CodeEditBox.MovePos(LOWORD(lParam) + 2, HIWORD(lParam) - 0);
	}
	break;
	case WM_SIZE:
	{
		int w = LOWORD(lParam) - 2 + GetSystemMetrics(SM_CXVSCROLL);
		int h = HIWORD(lParam) + 2 + GetSystemMetrics(SM_CYHSCROLL);
		Tesh.CodeEditBox.SetSize(w,h);
	}
	break;
	default:
		break;
	}
	return  result;
}


TESH::TESH() {
	TeshMenu = NULL;
	TriggerEditor=NULL;
	Edit = NULL;
	next_convert_to_code_dialog = FALSE;
	TeshModule = this;
}
TESH::~TESH() {
	TeshMenu = NULL;
	TriggerEditor = NULL;
	Edit = NULL;
	next_convert_to_code_dialog = FALSE;
	if (this == TeshModule) {
		TeshModule = NULL;
	}
	
}

void TESH::Resize() {
	if (!CodeEditBox._hwnd) { return; }
	CodeEditBox.Resize();
}

BOOL TESH::MakeMenu(HMENU hMenu) {
	TeshMenu = CreateMenu();
	AppendMenuA(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)TeshMenu,"TESH");
	AppendMenuA(TeshMenu, MF_STRING, 10010, "设置");
	AppendMenuA(TeshMenu, MF_STRING , 10020, "函数");
	AppendMenuA(TeshMenu, MF_SEPARATOR, 0, "");
	AppendMenuA(TeshMenu, MF_STRING, 10030, "关于");
	return TRUE;
}

void TESH::InitCodeEditRange() {
	if (!TriggerEditor || !Edit) {
		return;
	}
	RECT rt;
	POINT pos;
	SIZE size;
	GetWindowRect(Edit,&rt);
	pos = { rt.left,rt.top };
	size = { rt.right,rt.bottom };
	ScreenToClient(TriggerEditor, &pos);
	ScreenToClient(TriggerEditor, (POINT*)&size);
	size.cx = size.cx - pos.x-2;
	size.cy = size.cy - pos.y+2;
	CodeEditBox.MovePos(pos.x+2,pos.y-0);
	CodeEditBox.SetSize(size.cx, size.cy);
}

BOOL TESH::SaveEditText() {
	if (TeshModule && TriggerEditor && Edit) {

		//printf("保存代码\n");
		BOOL result = FALSE;
		char* text = TeshModule->CodeEditBox.GetEditPutText();
		if (text) {
			result = SetWindowTextA(Edit, text);
			free(text);
		}
		else {
			result = SetWindowTextA(Edit, "");
		}
		TeshModule->CodeEditBox._edit_state = 0;
		SendMessageA(TriggerEditor, WM_COMMAND, MAKELONG(17, 1024), (LPARAM)Edit);
		SendMessageA(TriggerEditor, WM_COMMAND, MAKELONG(17, 768), (LPARAM)Edit);
		return TRUE;
	}
	return FALSE;
}
void TESH::ResumeWorldEditor() {
	if (WorldEditorMain && pWE_WndProc) {
		SetWindowLongA(WorldEditorMain, GWL_WNDPROC, (LONG)&pWE_WndProc);
		pWE_WndProc = NULL;
	}
}
void TESH::ResumeTriggerEditor() {
	if (TriggerEditor && pTE_WndProc) {
		SetWindowLongA(TriggerEditor, GWL_WNDPROC, (LONG)&pTE_WndProc);
		pTE_WndProc = NULL;
	}
	if (Edit && pEdit_WndProc) {
		SetWindowLongA(Edit, GWL_WNDPROC, (LONG)&pEdit_WndProc);
		pEdit_WndProc = NULL;
	}
}
BOOL TESH::Init() {
	//预加载,防止打开触发器卡住。
	//需保持顺序!
	_Options.Init(NULL, (uintptr_t)&CodeEditBox, (uintptr_t)&FuncListWin, &config, &styles);
	if (!FuncListWin.Init(NULL)) {
		return FALSE;
	}
	FuncListWin._codebox.SetFuncKeywords(JassCjKeywords, JassBjKeywords, JassConstantKeywords);
	FuncListWin.LoadFuncList();
	return TRUE;
}
BOOL TESH::InitTriggerEditor(HWND parent, int nCmdShow) {
	if (TriggerEditor == NULL) {
		char title[50] = { 0 };
		GetWindowTextA(parent, title, 50);
		const char* name = config.get_str("TriggerEditor", "name");
		if (!name) {
			name = "触发编辑器";
		}
		config.set("TriggerEditor", "name", name);
		if (strcmp(title, name) != 0) {
			return FALSE;
		}
		if (!FuncListWin.Init(parent)) {
			return FALSE;
		}
		if (!CodeEditBox.Init(parent, &_Options)) {
			return FALSE;
		}
		CodeEditBox.FuncList = &FuncListWin;
		CodeEditBox.EditFocusCallback = CodeEditFocus;
		CodeEditBox.SetFuncKeywords(JassCjKeywords, JassBjKeywords, JassConstantKeywords);
		
		_Options.InitAppConfig(true);
		_Options.SaveConfig(true);

		TriggerEditor = parent;
		MakeMenu(GetMenu(TriggerEditor));
		pTE_WndProc = (WNDPROC)GetWindowLongA(TriggerEditor, GWL_WNDPROC);
		SetWindowLongA(TriggerEditor, GWL_WNDPROC, (LONG)&TE_WndProc);

		/*TE_TrigTreeView = GetChildCtrl(TriggerEditor,13);*/
		//=========================About===========================
		_TeshAbout.Init(NULL, "TESH - 触发编辑器 语法高亮 重制 版本 1.10 (dll插件)"
			, "代码高亮来源库:Scintilla,Lexilla"
			, "", "原作者:Filip Stanisavljevic(SPilip)\n作者和协助者: 123456 ...\n");
		//=========================================================
		return TRUE;
	}
	return TRUE;
}
BOOL TESH::InitEditText(HWND parent, int nCmdShow) {
	if (TriggerEditor == NULL) {
		InitTriggerEditor(parent, nCmdShow);
		return FALSE;
	}
	if (Edit == NULL) {
		if (GetParent(parent) == TriggerEditor && GetDlgCtrlID(parent) == 17) {
			Edit = parent;
			WINDOWPLACEMENT plac;
			GetWindowPlacement(Edit, &plac);
			SetWindowPlacement(CodeEditBox._main, &plac);
			pEdit_WndProc = (WNDPROC)GetWindowLongA(Edit, GWL_WNDPROC);
			SetWindowLongA(Edit, GWL_WNDPROC, (LONG)&Edit_WndProc);
			InitCodeEditRange();
			//FixIme();
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL TESH::ShowEditText(HWND parent, int nCmdShow) {
	if (!InitEditText(parent, nCmdShow)) {
		return FALSE;
	}
	if (parent == Edit) {
		return ShowWindow(CodeEditBox._main, nCmdShow);
	}
	return FALSE;
}
void TESH::ReadJassText() {
	std::string path;
	jass_analys analys;
	dllself.getfilepath(path, "common.j");
	analys.read(((char*)path.c_str()), cj_globals_text, cj_text);
	dllself.getfilepath(path, "Blizzard.j");
	analys.read(((char*)path.c_str()), bj_globals_text, bj_text);
	dllself.getfilepath(path, "scripts\\");

	char filename[MAX_PATH];
	strcpy_s(filename, path.c_str());
	strcat_s(filename, "*.j");    // 在目录后面加上"\\*.*"进行第一次搜索

	intptr_t handle;
	_finddata_t fileinfo;
	handle = _findfirst(filename, &fileinfo);
	if (handle != -1)        // 检查是否成功
	{
		do
		{
			if (fileinfo.attrib &  _A_SUBDIR
				|| strcmp(fileinfo.name, ".") == 0
				|| strcmp(fileinfo.name, "..") == 0
				|| strcmp(fileinfo.name, "common.j") == 0
				|| strcmp(fileinfo.name, "Blizzard.j") == 0
				|| strcmp(fileinfo.name, "common.ai") == 0)
			{
				continue;
			}
			strcpy_s(filename, path.c_str());
			strcat_s(filename, fileinfo.name);
			analys.read(filename, custom_globals_text, custom_function_text);

		} while (!_findnext(handle, &fileinfo));

		_findclose(handle);    // 关闭搜索句柄
	}
	
	JassFuncKeywords.init();
}
void TESH::ReadConfig() {
	std::string path = dllself.getfilepath("TESH.ini");
	config.open(path.c_str(), true);
	path = dllself.getfilepath("Styles.ini");
	styles.open(path.c_str(), true);
}

void TESH::InitConfig() {
	int color = 0;
	config._default["TriggerEditor"]["name"] = "触发编辑器";
	config._default["Editor"]["DoFold"] = 1;
	config._default["Editor"]["LineNum"] = 1;
	config._default["Editor"]["AutoComplete"] = 1;
	config._default["Editor"]["CallTip"] = 1;
	config._default["Editor"]["ZoomLevel"] = 1;
	config._default["Editor"]["TabLine"] = 0;
	config._default["Editor"]["AutoFillSpace"] = 1;
	config._default["Editor"]["AlwaysFoldAll"] = 0;
	config._default["Editor"]["SaveScrollPos"] = 1;
	config._default["Editor"]["Backdrop"] = "";
	config._default["Other"]["EnableUTF8"] = 1;
	config._default["Other"]["EnableD2dDraw"] = 1;
	config._default["Other"]["EnableAntiAlias"] = 2; config._default["Other"]["EnableAntiAlias"].comment = "//抗锯齿=1,ClearType抗锯齿=2";
	config._default["Other"]["ListOnTop"] = 1;
	config._default["Other"]["ShowWarning"] = 0;
	config._default["Other"]["SkipIncompatibleIme"] = 0;
	config._default["Other"]["ShowAnyPopCtrlAllSkipIncIme"] = 0;
	config._default["EditorFont"]["Name"] = "Courier New";
	config._default["EditorFont"]["Size"] = 10;
	config._default["LineNumFont"]["Name"] = "Courier New";
	config._default["LineNumFont"]["Size"] = 10;
	color = GetSysColor(COLOR_MENU);
	styles._default["Line Numbers"]["Foreground"] = (uint32_t)0x000000;
	styles._default["Line Numbers"]["Background"] = (uint32_t)TO_BGR(color);
	styles._default["FoldMarker"]["Foreground"] = (uint32_t)0x00FFFF;
	styles._default["FoldMarker"]["Background"] = (uint32_t)0x0000FF;
	styles._default["Caret"]["Background"] = (uint32_t)0xFFFF00;
	styles._default["Caret"]["Alpha"] = 60;
	styles._default["Caret"]["Visible"] = 0;
	styles._default["Selection"]["Foreground"] = (uint32_t)0xFFFFFF;
	styles._default["Selection"]["Background"] = (uint32_t)0x3399FF;
	styles._default["Selection"]["Alpha"] = 256;
	styles._default["SearchRange"]["Background"] = (uint32_t)0x666666;
	styles._default["SearchRange"]["Alpha"] = 50;
	styles._default["AnchorForWord"]["Background"] = 0x999999;
	styles._default["AnchorForWord"]["Alpha"] = 50;
	styles._default["AnchorForWord"]["Visible"] = 0;
	styles._default["Other"]["Foreground"] = (uint32_t)0x000000;
	styles._default["Other"]["Background"] = (uint32_t)0xFFFFFF;

	styles._default["Operator"]["Foreground"] = (uint32_t)0xAA5500;
	styles._default["Operator"]["Background"] = (uint32_t)0xFFFFFF;
	styles._default["Operator"]["Bold"] = 1;
	styles._default["Operator"]["Italic"] = 0;
	styles._default["Operator"]["Underline"] = 0;

	styles._default["Block"]["Foreground"] = "default";
	styles._default["Block"]["Background"] = "default";
	styles._default["Block"]["Bold"] = 1;
	styles._default["Block"]["Italic"] = 0;
	styles._default["Block"]["Underline"] = 0;

	styles._default["Keyword"]["Foreground"] = "default";
	styles._default["Keyword"]["Background"] = "default";
	styles._default["Keyword"]["Bold"] = 1;
	styles._default["Keyword"]["Italic"] = 0;
	styles._default["Keyword"]["Underline"] = 0;

	styles._default["Value"]["Foreground"] = "default";
	styles._default["Value"]["Background"] = "default";
	styles._default["Value"]["Bold"] = 1;
	styles._default["Value"]["Italic"] = 0;
	styles._default["Value"]["Underline"] = 0;

	styles._default["Type"]["Foreground"] = (uint32_t)0x0055AA;
	styles._default["Type"]["Background"] = "default";
	styles._default["Type"]["Bold"] = 1;
	styles._default["Type"]["Italic"] = 0;
	styles._default["Type"]["Underline"] = 0;

	styles._default["Constant"]["Foreground"] = (uint32_t)0x0000AA;
	styles._default["Constant"]["Background"] = "default";
	styles._default["Constant"]["Bold"] = 0;
	styles._default["Constant"]["Italic"] = 0;
	styles._default["Constant"]["Underline"] = 0;

	styles._default["Native"]["Foreground"] = (uint32_t)0x800000;
	styles._default["Native"]["Background"] = "default";
	styles._default["Native"]["Bold"] = 0;
	styles._default["Native"]["Italic"] = 0;
	styles._default["Native"]["Underline"] = 0;

	styles._default["BJ"]["Foreground"] = (uint32_t)0xFF0000;
	styles._default["BJ"]["Background"] = "default";
	styles._default["BJ"]["Bold"] = 0;
	styles._default["BJ"]["Italic"] = 0;
	styles._default["BJ"]["Underline"] = 0;

	styles._default["Number"]["Foreground"] = (uint32_t)0x0000AA;
	styles._default["Number"]["Background"] = "default";
	styles._default["Number"]["Bold"] = 0;
	styles._default["Number"]["Italic"] = 0;
	styles._default["Number"]["Underline"] = 0;

	styles._default["String"]["Foreground"] = (uint32_t)0x0000AA;
	styles._default["String"]["Background"] = "default";
	styles._default["String"]["Bold"] = 0;
	styles._default["String"]["Italic"] = 1;
	styles._default["String"]["Underline"] = 0;

	styles._default["CharNumber"]["Foreground"] = (uint32_t)0x000000;
	styles._default["CharNumber"]["Background"] = "default";
	styles._default["CharNumber"]["Bold"] = 1;
	styles._default["CharNumber"]["Italic"] = 0;
	styles._default["CharNumber"]["Underline"] = 1;

	styles._default["Runtextmacro"]["Foreground"] = (uint32_t)0x777777;
	styles._default["Runtextmacro"]["Background"] = "default";
	styles._default["Runtextmacro"]["Bold"] = 0;
	styles._default["Runtextmacro"]["Italic"] = 1;
	styles._default["Runtextmacro"]["Underline"] = 0;

	styles._default["Comment"]["Foreground"] = (uint32_t)0x008800;
	styles._default["Comment"]["Background"] = "default";
	styles._default["Comment"]["Bold"] = 0;
	styles._default["Comment"]["Italic"] = 1;
	styles._default["Comment"]["Underline"] = 0;

	styles._default["Preprocessor Comment"]["Foreground"] = (uint32_t)0x000000;
	styles._default["Preprocessor Comment"]["Background"] = "default";
	styles._default["Preprocessor Comment"]["Bold"] = 1;
	styles._default["Preprocessor Comment"]["Italic"] = 0;
	styles._default["Preprocessor Comment"]["Underline"] = 0;

	styles._default["Lua Operator"]["Foreground"] = (uint32_t)0xAA5500;
	styles._default["Lua Operator"]["Background"] = "default";
	styles._default["Lua Operator"]["Bold"] = 1;
	styles._default["Lua Operator"]["Italic"] = 0;
	styles._default["Lua Operator"]["Underline"] = 0;

	styles._default["Lua Keyword"]["Foreground"] = "default";
	styles._default["Lua Keyword"]["Background"] = "default";
	styles._default["Lua Keyword"]["Bold"] = 1;
	styles._default["Lua Keyword"]["Italic"] = 0;
	styles._default["Lua Keyword"]["Underline"] = 0;

	styles._default["Lua JassConstant"]["Foreground"] = (uint32_t)0x0000AA;
	styles._default["Lua JassConstant"]["Background"] = "default";
	styles._default["Lua JassConstant"]["Bold"] = 0;
	styles._default["Lua JassConstant"]["Italic"] = 0;
	styles._default["Lua JassConstant"]["Underline"] = 0;

	styles._default["Lua JassNative"]["Foreground"] = (uint32_t)0x800000;
	styles._default["Lua JassNative"]["Background"] = "default";
	styles._default["Lua JassNative"]["Bold"] = 0;
	styles._default["Lua JassNative"]["Italic"] = 0;
	styles._default["Lua JassNative"]["Underline"] = 0;

	styles._default["Lua JassBJ"]["Foreground"] = (uint32_t)0xFF0000;
	styles._default["Lua JassBJ"]["Background"] = "default";
	styles._default["Lua JassBJ"]["Bold"] = 0;
	styles._default["Lua JassBJ"]["Italic"] = 0;
	styles._default["Lua JassBJ"]["Underline"] = 0;

	styles._default["Lua Number"]["Foreground"] = (uint32_t)0x0000AA;
	styles._default["Lua Number"]["Background"] = "default";
	styles._default["Lua Number"]["Bold"] = 0;
	styles._default["Lua Number"]["Italic"] = 0;
	styles._default["Lua Number"]["Underline"] = 0;

	styles._default["Lua String"]["Foreground"] = (uint32_t)0xCC6600;
	styles._default["Lua String"]["Background"] = "default";
	styles._default["Lua String"]["Bold"] = 0;
	styles._default["Lua String"]["Italic"] = 0;
	styles._default["Lua String"]["Underline"] = 0;

	styles._default["Lua String2"]["Foreground"] = (uint32_t)0xCC6600;
	styles._default["Lua String2"]["Background"] = "default";
	styles._default["Lua String2"]["Bold"] = 0;
	styles._default["Lua String2"]["Italic"] = 0;
	styles._default["Lua String2"]["Underline"] = 0;

	styles._default["Lua LiteralString"]["Foreground"] = (uint32_t)0xCC6600;
	styles._default["Lua LiteralString"]["Background"] = "default";
	styles._default["Lua LiteralString"]["Bold"] = 0;
	styles._default["Lua LiteralString"]["Italic"] = 0;
	styles._default["Lua LiteralString"]["Underline"] = 0;

	styles._default["Lua Comment"]["Foreground"] = (uint32_t)0x008800;
	styles._default["Lua Comment"]["Background"] = "default";
	styles._default["Lua Comment"]["Bold"] = 0;
	styles._default["Lua Comment"]["Italic"] = 1;
	styles._default["Lua Comment"]["Underline"] = 0;

	ReadConfig();
	if ((Tesh.config.get_int("Other", "SkipIncompatibleIme", 1) == 1)) {
		FixIme();
	}
	ShowAnyPopCtrlAllSkipIncIme = (Tesh.config.get_int("Other", "ShowAnyPopCtrlAllSkipIncIme", 1) == 1);
}

void TESH::InitGlobals() {
	DuiLibSetInstanceInit((HINSTANCE)GetModuleHandle(NULL));
	InitConfig();
	ReadJassText();
	InitIATHook();
	Tesh.Init();
}
void TESH::Destroy() {
	if (Tesh.TriggerEditor) {
		Tesh._Options.SaveConfig(true);
	}
	ResumeWorldEditor();
	ResumeTriggerEditor();
	UninstallIATHook();
}
