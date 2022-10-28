#pragma once
#include <windows.h>
#include <stdint.h>

#include <tesh.h>
extern uintptr_t pTrueCreateWindowExA;
extern uintptr_t pTrueSetMenu;
extern uintptr_t pTrueShowWindow;
extern uintptr_t pTrueSetWindowTextA;
extern uintptr_t pTrueDestroyWindow;
extern uintptr_t pTrueMessageBoxA;
extern uintptr_t pTruePeekMessageA;

#define DetourWeCreateWindowExA TESH::DetourWeCreateWindowExA
#define DetourWeSetMenu TESH::DetourWeSetMenu
#define DetourWeShowWindow TESH::DetourWeShowWindow
#define DetourWeSetWindowTextA TESH::DetourWeSetWindowTextA
#define DetourWeDestroyWindow TESH::DetourWeDestroyWindow
#define DetourWeMessageBoxA TESH::DetourWeMessageBoxA
#define DetourWePeekMessageA TESH::DetourWePeekMessageA

