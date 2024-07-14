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

/**
 * @brief Construct a new Lin Inject:: Lin Inject object
 * @param dllName -> The name of the DLL to inject
 * @param processName -> The name of the process to inject the DLL into
 */
LinInject::LinInject(const char* dllName, const char* processName)
    : m_dllName(dllName), m_processName(processName), m_targetProcess(0), m_pathAddress(nullptr), m_dlopenAddress(nullptr)
{
}

/**
 * @brief Destroy the Lin Inject:: Lin Inject object
 */
LinInject::~LinInject()
{
    if (m_pathAddress != nullptr)
    {
        munmap(m_pathAddress, sizeof(m_dllName) + 1);
    }
}

/**
 * @brief Injects the shared object into the target process
 * @return std::vector<Result> -> A vector of results for each step of the injection process
 */
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

/**
 * @brief Find the process ID of the target process
 * @param processName -> The name of the target process
 * @param processId -> The process ID of the target process
 * @return std::vector<Result> -> A vector of results for each step of the operation
 */
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

/**
 * @brief Open a handle to the target process
 * @param processId -> The process ID of the target process
 * @return Result -> The result of the operation
 */
Result LinInject::OpenProcessHandle(pid_t processId)
{
    m_targetProcess = processId;
    return {true, "Process handle opened successfully"};
}

/**
 * @brief Allocate memory in the target process and write the DLL path to it
 * @return Result -> The result of the operation
 */
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

/**
 * @brief Get the address of the dlopen function in the target process
 * @return Result -> The result of the operation
 */
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

/**
 * @brief Inject the shared library into the target process
 * @return Result -> The result of the operation
 */
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

/**
 * @brief Print the results of the injection process
 * @param results -> The results of the injection process
 */
void LinInject::PrintResults(const std::vector<Result>& results)
{
    for (const auto& result : results)
    {
        std::cout << (result.success ? "Success: " : "Error: ") << result.message << std::endl;
    }
}

/**
 * @brief Convert a block of memory to a hex string
 * @param data -> The block of memory
 * @param size -> The size of the block of memory
 * @return std::string -> The hex string
 */
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