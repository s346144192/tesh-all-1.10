// dllmain.cpp : 定义 DLL 应用程序的入口点。
#define no_init_all deprecated

#include <windows.h>
#include <dll_info.h>
#include <tesh.h>
#include<base/util/console.h>



//windows 10 微软输入法不兼容,偶尔在切换自定义代码时会卡住。

/*
尚未完成的功能:
1.鼠标悬停于函数显示注释。
2.自动完成和函数提示控件显示时跟随编辑窗口移动位置。


*/


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		dllself(hModule);
		TESH::InitGlobals();
		//base::console::enable();
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		TESH::Destroy();
        break;
    }
    return TRUE;
}

