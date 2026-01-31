#ifndef BAD_DLL_H
#define BAD_DLL_H

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#  define DLL_EXPORT __declspec(dllexport)
#else
#  define DLL_EXPORT
#endif

DLL_EXPORT BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

#ifdef __cplusplus
}
#endif
#endif
#endif
