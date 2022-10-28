// test_tesh.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include <io.h>
#include "test_tesh.h"
#include <Scintilla.h>
#include <SciLexer.h>
#include <dll_info.h>
#include <text/jassmap2.h>
#include <text/jassmap3.h>

#include <DuiLibLoader.h>


#define MAX_LOADSTRING 100

TESH tesh;
ini::config_t TESH::config, TESH::styles;
jass_map cj_text, bj_text, custom_function_text;
jass_map cj_globals_text, bj_globals_text, custom_globals_text;

#pragma  warning (disable:4996)//屏蔽不安全提示
VOID open_console()
{
	static BOOL console_enable = FALSE;
	if (console_enable) {
		return;
	}
	console_enable = TRUE;
	//-------控制台用的--------------
	HINSTANCE g_hInstance = 0;
	HANDLE g_hOutput = 0;
	HWND hwnd = NULL;
	HMENU hmenu = NULL;
	CHAR title[] = "测试窗口";
	HANDLE hdlWrite = NULL;
	//---------------------
	//------------------控制台-----------------------
	AllocConsole();
	freopen("CONOUT$", "w+t", stdout);
	freopen("CONIN$", "r+t", stdin);
	g_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	//设置控制台窗口的属性
	SetConsoleTitle(title);
	SetConsoleTextAttribute((HANDLE)g_hOutput, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	while (NULL == hwnd) hwnd = ::FindWindow(NULL, (LPCTSTR)title);
	//屏蔽掉控制台窗口的关闭按钮，以防窗口被误删除
	hmenu = ::GetSystemMenu(hwnd, FALSE);
	//DeleteMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
	hdlWrite = GetStdHandle(STD_OUTPUT_HANDLE); //这里也可以使用STD_ERROR_HANDLE    TCHAR c[] = {"Hello world!"};WriteConsole(hdlWrite, c, sizeof(c), NULL, NULL);
}


// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTTESH, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTTESH));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTTESH));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TESTTESH);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//



void TESH::ReadJassText() {
	std::string path;
	jass_analys analys;
	//jass_analys_scaner analys2;
	uint32_t out_time = GetTickCount();

	/*dllself.getfilepath(path, "war3map.j");
	fpfile fp(path.c_str(),"rt");
	fp.read_buffer();
	printf("打开文件:%dms\n", GetTickCount() - out_time);
	
	analys2.load(fp.buffer);
	out_time = GetTickCount();
	for (int i = 1; i <= 100;i++) {
		analys2._pos = analys2._text;
		analys2.init_wordlist();
		printf("词法分解进度:%d/100\n", i);
	}
	printf("词法分解:%dms\n", GetTickCount() - out_time);
	analys2.save_buffer_tofile("scaner.out");
	int i = analys2.wordlist.size();
	out_time = GetTickCount();
	fp.close();
	printf("关闭文件:%dms\n", GetTickCount() - out_time);*/

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
	if (handle == -1)        // 检查是否成功
		return;
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

	JassFuncKeywords.init();
	CodeEditBox.SetFuncKeywords(JassCjKeywords, JassBjKeywords, JassConstantKeywords);
	FuncListWin._codebox.SetFuncKeywords(JassCjKeywords, JassBjKeywords, JassConstantKeywords);

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

	config._default["Other"]["EnableUTF8"] = 0;
	config._default["Other"]["EnableD2dDraw"] = 1; config._default["Other"]["EnableD2dDraw"].comment = "//也许能提高效率";
	config._default["Other"]["EnableAntiAlias"] = 2; config._default["Other"]["EnableAntiAlias"].comment = "//抗锯齿=1,ClearType抗锯齿=2";
	
	config._default["Other"]["ListOnTop"] = 1;
	config._default["Other"]["ShowWarning"] = 1;
	config._default["Other"]["SkipIncompatibleIme"] = 1;
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
	styles._default["AnchorForWord"]["Background"] = (uint32_t)0x999999;
	styles._default["AnchorForWord"]["Alpha"] = 50;
	styles._default["AnchorForWord"]["Visible"] = 1;
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

	styles._default["Preprocessor Comment"]["Foreground"] = (uint32_t)0x777777;
	styles._default["Preprocessor Comment"]["Background"] = "default";
	styles._default["Preprocessor Comment"]["Bold"] = 0;
	styles._default["Preprocessor Comment"]["Italic"] = 1;
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

	styles._default["Lua String"]["Foreground"] = (uint32_t)0xAA8000;
	styles._default["Lua String"]["Background"] = "default";
	styles._default["Lua String"]["Bold"] = 0;
	styles._default["Lua String"]["Italic"] = 0;
	styles._default["Lua String"]["Underline"] = 0;

	styles._default["Lua String2"]["Foreground"] = (uint32_t)0xAA8000;
	styles._default["Lua String2"]["Background"] = "default";
	styles._default["Lua String2"]["Bold"] = 0;
	styles._default["Lua String2"]["Italic"] = 0;
	styles._default["Lua String2"]["Underline"] = 0;

	styles._default["Lua LiteralString"]["Foreground"] = (uint32_t)0xAA8000;
	styles._default["Lua LiteralString"]["Background"] = "default";
	styles._default["Lua LiteralString"]["Bold"] = 0;
	styles._default["Lua LiteralString"]["Italic"] = 0;
	styles._default["Lua LiteralString"]["Underline"] = 0;

	styles._default["Lua Comment"]["Foreground"] = (uint32_t)0x008800;
	styles._default["Lua Comment"]["Background"] = "default";
	styles._default["Lua Comment"]["Bold"] = 0;
	styles._default["Lua Comment"]["Italic"] = 1;
	styles._default["Lua Comment"]["Underline"] = 0;

	std::string path = dllself.getfilepath("TESH.ini");
	config.open(path.c_str(), true);
	path = dllself.getfilepath("Styles.ini");
	styles.open(path.c_str(), true);

}
void TESH::Resize() {
	if (!CodeEditBox._hwnd) { return; }
	CodeEditBox.Resize();
}


BOOL TESH::Init(HWND parent) {
	InitConfig();
	_Options.Init(parent, (uintptr_t)&CodeEditBox, (uintptr_t)&FuncListWin, &config, &styles);
	if (!FuncListWin.Init(parent)) {
		return FALSE;
	}
	if (!CodeEditBox.Init(parent,&_Options)) {
		return FALSE;
	}
	CodeEditBox.FuncList = &FuncListWin;
	_Options.InitAppConfig(true);
	_Options.SaveConfig(true);
	ReadJassText();
	FuncListWin.LoadFuncList();
	_TeshAbout.Init(parent, "TESH - 触发编辑器 语法高亮 重制 版本 1.0 (dll插件)"
		, "说明:\n需注意切换Shift键可能使输入无效"
		,"1638048450@qq.com"
		, "作者和协助者: 云龙 ...\n");
	return TRUE;
}


BOOL InitScilexer(HWND parent) {
	tesh.Init(parent);
	return TRUE;
}

void testkey() {
	DWORD InitTime = GetTickCount();
	InitTime = GetTickCount();
	srand(1);
	printf("GetKeyState测试开始\n");
	for (int i = 0; i < 100000; i++) {
		GetKeyState(rand() % 256);
	};
	InitTime = GetTickCount() - InitTime;
	printf("GetKeyState时间:%d\n", InitTime);

	InitTime = GetTickCount();
	srand(2);
	printf("GetAsyncKeyState测试开始\n");
	for (int i = 0; i < 100000; i++) {
		GetAsyncKeyState(rand() % 256);
	};
	InitTime = GetTickCount() - InitTime;
	printf("GetAsyncKeyState时间:%d\n", InitTime);
	InitTime = GetTickCount();
	srand(3);
	printf("条件测试开始\n");
	int count = 0;
	for (int i = 0; i < 100000; i++) {
		if (rand() == 1 || rand() == 11113) {
			count++;
		}
		else if (rand() == 5 || rand() == 2222) {
			count++;
		};
	};
	InitTime = GetTickCount() - InitTime;
	printf("条件时间:%d\n", InitTime);

	InitTime = GetTickCount();
	srand(4);
	printf("HWND速度测试开始\n");
	count = 0;
	for (int i = 0; i < 100000; i++) {
		if ((HWND)rand() == (HWND)1 || (HWND)rand() == (HWND)155511) {
			count++;
		}
		else if ((HWND)rand() == (HWND)5 || (HWND)rand() == (HWND)2222) {
			count++;
		};
	};
	InitTime = GetTickCount() - InitTime;
	printf("HWND速度测试时间:%d\n", InitTime);
}

float GetRadDiff(float r1,float r2) {
	float diff = r1 - r2;
	if (diff<-3.1415926) {
		return -(3.1415926 - (-diff - 3.1415926));
	}
	else if (diff>3.1415926) {
		return 3.1415926 - (diff - 3.1415926);
	}
	return diff;
}



Timer timer1;

void TimerFunc1(int args1, int args2) {
	printf("args:%d,%d\n", args1, args2);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   open_console();
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   DWORD InitTime = GetTickCount();
   InitScilexer(hWnd);
   InitTime = GetTickCount()- InitTime;
   //HMODULE module=LoadLibraryA("Duilib.dll");
   DuiLibSetInstanceInit(hInstance);
   //float r1 = -2.0;
   //float r2 = 2.0;
   //for (int i = 0; i < 31;i++) {
	  // r1 -= 0.1;
	  // r2 += 0.1;
	  // printf("%f,%f 弧度之差:%f\n",r1,r2, GetRadDiff(r1,r2));
   //}

   //int n = 0;
   //for (int i = 0; i <= 0xFF;++i) {
	  // n = -i & 0xFF;
	  // printf("%d & 0xFF = %d << 1 = %d & 2 = %d\n",-i,-i & 0xFF, (n << 1), ((n << 1) & 3));
   //}
   //testkey();
   //printf("初始化时间: %d ms\n",InitTime);
   /*tesh.ReadJassText();*/

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_KEYDOWN: {
		if (GetKeyState(VK_CONTROL) < 0)
		{
			switch (wParam)
			{
			case 'Z':
				break;
			case 'Y':
				break;
			case 'X':
				break;
			case 'C':
				break;
			case 'V':
				break;
			case 'A':
				break;
			case 'F':
				tesh.CodeEditBox._findtext.show(true,true);
				break;
			case 'H':
				tesh.CodeEditBox._findtext.show(true, false);
				break;
			default:
				break;
			}
		}
	}
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_MOVE:
	{
		if (tesh.CodeEditBox._hwnd) {
			tesh.CodeEditBox.AutoCompleteUpdate();
			tesh.CodeEditBox.SetCallTipUpdate();
		}
		
	}
	break;
    case WM_SIZE:
        {
			tesh.Resize();
        }
        break;
	case WM_NOTIFY:
		{
		}
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
