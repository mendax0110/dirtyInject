#include "../../include/linInject.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>

#if defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <fcntl.h>
#include <dlfcn.h>

LinInject::LinInject(const char* dllName, const char* processName)
    : m_dllName(dllName), m_processName(processName), m_targetProcess(0), m_pathAddress(nullptr), m_dlopenAddress(nullptr)
{
}

LinInject::~LinInject()
{
    if (m_pathAddress != nullptr)
    {
        munmap(m_pathAddress, sizeof(m_dllName) + 1);
    }
}

std::vector<Result> LinInject::InjectSo()
{
    std::vector<Result> results;
    pid_t processId;

    results = FindProcessId(m_processName.c_str(), processId);
    if (!results.back().success)
    {
        return results;
    }

    results.push_back(OpenProcessHandle(processId));
    if (!results.back().success)
    {
        return results;
    }

    results.push_back(AllocateAndWriteMemory());
    if (!results.back().success)
    {
        return results;
    }

    results.push_back(GetDlopenAddress());
    if (!results.back().success)
    {
        return results;
    }

    results.push_back(InjectSharedLibrary());
    if (!results.back().success)
    {
        return results;
    }

    results.push_back({true, "DLL injected successfully"});
    return results;
}

std::vector<Result> LinInject::FindProcessId(const char* processName, pid_t& processId)
{
    std::vector<Result> results;
    int fd;
    char buf[4096], *p;
    pid_t pid = 0;
    size_t n;

    results.push_back({false, "Not yet implemented"});
    return results;
}

Result LinInject::OpenProcessHandle(pid_t processId)
{
    m_targetProcess = processId;
    return {true, "Process handle opened successfully"};
}

Result LinInject::AllocateAndWriteMemory()
{
    m_pathAddress = mmap(0, sizeof(m_dllName) + 1, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (m_pathAddress == MAP_FAILED)
    {
        return {false, "Failed to allocate memory in the process"};
    }

    std::ostringstream memoryAllocatedMessage;
    memoryAllocatedMessage << "Memory allocated at: " << std::hex << reinterpret_cast<uintptr_t>(m_pathAddress);

    strcpy((char*)m_pathAddress, m_dllName.c_str());

    std::ostringstream memoryWrittenMessage;
    memoryWrittenMessage << "DLL path written to process memory: " << m_dllName;

    return {true, memoryAllocatedMessage.str() + "\n" + memoryWrittenMessage.str()};
}

Result LinInject::GetDlopenAddress()
{
    m_dlopenAddress = dlsym(RTLD_NEXT, "dlopen");
    if (m_dlopenAddress == nullptr)
    {
        return {false, "Failed to get the address of dlopen"};
    }

    std::ostringstream dlopenAddressMessage;
    dlopenAddressMessage << "dlopen address: " << m_dlopenAddress;

    return {true, dlopenAddressMessage.str()};
}

Result LinInject::InjectSharedLibrary()
{
    std::ostringstream command;
    command << "LD_PRELOAD=" << m_dllName << " ";
    command << "/proc/" << m_targetProcess << "/exe";

    if (system(command.str().c_str()) == -1)
    {
        return {false, "Failed to inject shared library"};
    }

    return {true, "Shared library injected successfully"};
}

void LinInject::PrintResults(const std::vector<Result>& results)
{
    for (const auto& result : results)
    {
        std::cout << (result.success ? "Success: " : "Error: ") << result.message << std::endl;
    }
}

std::string LinInject::HexDump(const void* data, size_t size)
{
    std::ostringstream dump;
    const unsigned char* bytes = static_cast<const unsigned char*>(data);

    for (size_t i = 0; i < size; i++)
    {
        dump << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]) << " ";
    }

    return dump.str();
}
#endif