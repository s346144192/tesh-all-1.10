#include <DuiLibLoader.h>
#include <DuiLib/UIlib.h>
#include <dll_info.h>

#ifdef _DEBUG
#   ifdef UILIB_STATIC
#		pragma comment(lib, "..\\duilib\\DuiLib\\Build\\Debug\\DuiLib_Static_d.lib")
#   else
#		ifdef _UNICODE
#			pragma comment(lib, "DuiLib_ud.lib")
#		else
#			pragma comment(lib, "DuiLib_d.lib")
#		endif
#   endif
#else
#   ifdef UILIB_STATIC
#		pragma comment(lib, "..\\duilib\\DuiLib\\Build\\Release\\DuiLib_Static.lib")
#	else
#		ifdef _UNICODE
#			pragma comment(lib, "DuiLib_u.lib")
#		else
#			pragma comment(lib, "DuiLib.lib")
#		endif
#   endif
#endif

//using namespace DuiLib;

BOOL DuiLibSetInstanceInit(HINSTANCE hInstance) {
	DuiLib::CPaintManagerUI::SetInstance(hInstance);

	if (!com_loader.isinit()) return FALSE;
	return TRUE;
}
