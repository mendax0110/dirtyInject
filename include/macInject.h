#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>

#if defined(__APPLE__)
#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <dlfcn.h>
#include <sys/sysctl.h>

struct Result
{
    bool success;
    std::string message;
};

class MacInject
{
public:
    MacInject(const char* dylibName, const char* processName);
    ~MacInject();
    std::vector<Result> InjectDylib();

    const char* m_dylibName;
    const char* m_processName;
    mach_port_t m_targetTask;
    mach_vm_address_t m_pathAddress;
    void* m_dlopenAddress;
    char m_fullDylibPath[PATH_MAX]{};

    static Result FindProcessId(const char* processName, pid_t& processId);
    Result OpenProcessHandle(pid_t processId);
    Result AllocateAndWriteMemory();
    Result GetDlopenAddress();
    Result CreateRemoteThreadToLoadDylib();
    static void PrintResults(const std::vector<Result>& results);
    static std::string HexDump(const void* data, size_t size);
};
#endif