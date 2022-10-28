#include <base/hook/fp_call.h>
#include <hookfunc.h>

uintptr_t pTrueCreateWindowExA = 0;
uintptr_t pTrueSetMenu = 0;
uintptr_t pTrueShowWindow = 0;
uintptr_t pTrueSetWindowTextA = 0;
uintptr_t pTrueDestroyWindow = 0;
uintptr_t pTrueMessageBoxA = 0;
uintptr_t pTruePeekMessageA = 0;
