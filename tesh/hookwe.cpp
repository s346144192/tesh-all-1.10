//#include <cstdlib>
//#include <cstring>
//#include <string>
//#include <algorithm>
//#include <functional>
//#include <regex>
//#include <base/util/unicode.h>
//#include <base/hook/inline.h>
#include <base/hook/iat_manager.h>
//#include <base/hook/fp_call.h>
//#include <base/util/format.h>
//#include <base/win/pe_reader.h>

//#include <objbase.h>
#include <windows.h>
//#include <Shlobj.h>
#include <MemoryPatch.h>

#include <hookfunc.h>
#include <hookwe.h>

base::hook::iat_manager WE_IAT_Hooker;
#define INSTALL_WE_IAT_HOOK(apiName) WE_IAT_Hooker.hook(#apiName, &pTrue##apiName##, (uintptr_t)DetourWe##apiName##)


bool InstallPatch(const char* name, uintptr_t address, uint8_t *patch, uint32_t patchLength)
{
	return MemoryPatchAndVerify((void*)address, patch, patchLength);
}

void InitIATHook() {
	if (WE_IAT_Hooker.open_module(::GetModuleHandleW(NULL)))
	{
		if (WE_IAT_Hooker.open_dll("user32.dll"))
		{
			
			//INSTALL_WE_IAT_HOOK(CreateWindowExA);
			//INSTALL_WE_IAT_HOOK(SetMenu);
			
			INSTALL_WE_IAT_HOOK(ShowWindow);
			INSTALL_WE_IAT_HOOK(SetWindowTextA);
			INSTALL_WE_IAT_HOOK(DestroyWindow);
			INSTALL_WE_IAT_HOOK(MessageBoxA);
			INSTALL_WE_IAT_HOOK(PeekMessageA);

			/*log_put("HOOK", "ShowWindow:%x  ", "", pTrueShowWindow);
			log_put("HOOK", "SetWindowTextA:%x  ", "", pTrueSetWindowTextA);
			log_put("HOOK", "DestroyWindow:%x  ", "", pTrueDestroyWindow);
			log_put("HOOK", "MessageBoxA:%x  ", "", pTrueMessageBoxA);
			log_put("HOOK", "PeekMessageA:%x  ", "", pTruePeekMessageA);*/

		}
	}
}

void UninstallIATHook()
{
	WE_IAT_Hooker.release();
}