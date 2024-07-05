#ifndef BAD_DLL_H
#define BAD_DLL_H

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

#endif
#endif
