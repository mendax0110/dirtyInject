#include "../../include/macInject.h"

#if defined(__APPLE__)
MacInject::MacInject(const char* dylibName, const char* processName)
    : m_dylibName(dylibName), m_processName(processName), m_targetProcess(0), m_pathAddress(nullptr), m_dlopenAddress(nullptr)
{
}

MacInject::~MacInject()
{
    if (m_pathAddress != 0)
    {
        mach_vm_deallocate(m_targetTask, m_pathAddress, strlen(m_fullDylibPath) + 1);
    }

    if (m_targeTask != MACH_PORT_NULL)
    {
        mach_port_deallocate(mach_task_self(), m_targeTask);
    }
}

std::vector<Result> MacInject::InjectDylib()
{
    std::vector<Result> results;
    pid_t processId;

    resuls.push_back(FindProcessId(m_processName, processId));
    if (!results.back().success)
    {
        return results;
    }

    if (realpath(m_dylibName, m_fullDylibPath) == nullptr)
    {
        results.push_back({ false, "Failed to get the full path of the dylib: " + std::string(m_dylibName) });
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

    results.push_back(CreateRemoteThreadToLoadDylib());
    if (!results.back().success)
    {
        return results;
    }

    results.push_back({ true, "Dylib injected successfully" });
    return results;
}

Result MacInject::FindProcessId(const char* processName, pid_t& processId)
{
    std::ostringstream cmd;
    cmd << "pgrep " << processName;
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (pipe == nullptr)
    {
        return { false, "Failed to execute pgrep" };
    }

    char buffer[128];
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        processId = atoi(buffer);
        pclose(pipe);
        return { true, "Found process ID: " + std::string(processName) + ", PID: " + std::to_string(processId) };
    }

    pclose(pipe);
    return { false, "Failed to find process ID: " + std::string(processName) };
}

Result MacInject::OpenProcessHandle(pid_t processId)
{
    kern_return_t kr = task_for_pid(mach_task_self(), processId, &m_targetTask);
    if (kr != KERN_SUCCESS)
    {
        return { false, "Failed to get task for PID: " + std::to_string(processId) };
    }

    return { true, "Opened process handle successfully" };
}

Result MacInject::AllocateAndWriteMemory()
{
    mach_vm_address_t address;
    mach_vm_size_t size = strlen(m_fullDylibPath) + 1;
    kern_return_t kr = mach_vm_allocate(m_targetTask, &address, size, VM_FLAGS_ANYWHERE);
    if (kr != KERN_SUCCESS)
    {
        return { false, "Failed to allocate memory in the target process" };
    }

    m_pathAddress = address;
    kr = mach_vm_write(m_targetTask, address, (vm_offset_t)m_fullDylibPath, size);
    if (kr != KERN_SUCCESS)
    {
        return { false, "Failed to write memory in the target process" };
    }

    return { true, "Allocated and wrote memory successfully" };
}

Result MacInject::GetDlopenAddress()
{
    m_dlopenAddress = dlsym(RTLD_DEFAULT, "dlopen");
    if (m_dlopenAddress == nullptr)
    {
        return { false, "Failed to get the address of dlopen" };
    }

    return { true, "Got the address of dlopen successfully"  + std::to_string(reinterpret_cast<uintptr_t>(m_dlopenAddress)) };
}

Result MacInject::CreateRemoteThreadToLoadDylib()
{
    thread_act_t thread;
    x86_thread_state64_t state;
    mach_msg_type_number_t stateCount = x86_THREAD_STATE64_COUNT;

    memset(&state, 0, sizeof(state));
    state.__rip = (uint64_t)m_dlopenAddress;
    state.__rdi = (uint64_t)m_pathAddress;
    state.__rsi = RTLD_NOW;

    kern_return_t kr = thread_create_running(m_targetTask, x86_THREAD_STATE64, (thread_state_t)&state, stateCount, &thread);
    if (kr != KERN_SUCCESS)
    {
        mach_vm_deallocate(m_targetTask, m_pathAddress, strlen(m_fullDylibPath) + 1);
        return { false, "Failed to create a remote thread" };
    }

    mach_msg_type_number_t thread_state_count = x86_THREAD_STATE64_COUNT;
    while (true)
    {
        kr = thread_get_state(thread, x86_THREAD_STATE64, (thread_state_t)&state, &thread_state_count);
        if (kr != KERN_SUCCESS || state.__rax != 0)
        {
            break;
        }
        usleep(100);
    }

    return { true, "Created remote thread successfully and dylib was injected" };
}


void MacInject::PrintResults(const std::vector<Result>& results)
{
    for (const Result& result : results)
    {
        std::cout << (result.success ? "[SUCCESS] " : "[ERROR] ") << result.message << std::endl;
    }
}

std::string MacInject::HexDump(const void* data, size_t size)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    for (size_t i = 0; i < size; i++)
    {
        oss << std::setw(2) << static_cast<unsigned>(reinterpret_cast<const uint8_t*>(data)[i]);
    }
    return oss.str();
}
#endif