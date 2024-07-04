#pragma once

#include <iostream>
#include <string>
#include <vector>

#if defined(__linux__)
#include <sys/types.h>

struct Result
{
    bool success;
    std::string message;
};

class LinInject
{
public:
    LinInject(const char* soName, const char* processName);
    ~LinInject();
    std::vector<Result> InjectSo();

    std::string m_soName;
    std::string m_processName;
    pid_t m_targetProcess;
    void* m_pathAddress;
    void* m_dlopenAddress;

    std::vector<Result> FindProcessId(const char* processName, pid_t& processId);
    Result OpenProcessHandle(pid_t processId);
    Result AllocateAndWriteMemory();
    Result GetDlopenAddress();
    Result InjectSharedLibrary();
    void PrintResults(const std::vector<Result>& results);
    std::string HexDump(const void* data, size_t size);
};
#endif