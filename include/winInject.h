#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <Tlhelp32.h>

struct Result
{
    bool success;
    std::string message;
};

class WinInject
{
public:
    WinInject(const char* dllName, const char* processName);
    ~WinInject();
    std::vector<Result> InjectDll();
    
    const char* m_dllName;
    const char* m_processName;
    HANDLE m_tragetProcess;
    void* m_pathAddress;
    void* m_loadLibraryAddress;
    char m_fullDllPath[MAX_PATH];

    Result FindProcessId(const char* processName, DWORD& processId);
    Result OpenProcessHandle(DWORD processId);
    Result AllocateAndWriteMemory();
    Result GetLoadLibraryAddress();
    Result CreateRemoteThreadToLoadLibrary();
    void PrintResults(const std::vector<Result>& results);
    std::string HexDump(const void* data, size_t size);
};
#endif