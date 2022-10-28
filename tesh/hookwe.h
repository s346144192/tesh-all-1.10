#pragma once
#include <stdint.h>


bool InstallPatch(const char* name, uintptr_t address, uint8_t *patch, uint32_t patchLength);

void InitIATHook();
void UninstallIATHook();
