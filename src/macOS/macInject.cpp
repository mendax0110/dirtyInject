#include <iomanip>
#include "../../include/macInject.h"

#if defined(__APPLE__)

/**
 * @brief Construct a new Mac Inject:: Mac Inject object
 * @param dylibName -> The name of the dylib to inject
 * @param processName -> The name of the process to inject the dylib into
 */
MacInject::MacInject(const char* dylibName, const char* processName)
        : m_dylibName(dylibName), m_processName(processName), m_targetTask(0), m_pathAddress(0), m_dlopenAddress(nullptr)
{
    memset(m_fullDylibPath, 0, sizeof(m_fullDylibPath));
}

/**
 * @brief Destroy the Mac Inject:: Mac Inject object
 */
MacInject::~MacInject()
{
    if (m_pathAddress != 0)
    {
        mach_vm_deallocate(m_targetTask, m_pathAddress, strlen(m_fullDylibPath) + 1);
    }

    if (m_targetTask != MACH_PORT_NULL)
    {
        mach_port_deallocate(mach_task_self(), m_targetTask);
    }
}

/**
 * @brief Injects the dylib into the target process
 * @return std::vector<Result> -> A vector of results for each step of the injection process
 */
std::vector<Result> MacInject::InjectDylib()
{
    std::vector<Result> results;
    pid_t processId;

    results.push_back(FindProcessId(m_processName, processId));
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

/**
 * @brief Finds the process ID of the target process
 * @param processName -> The name of the target process
 * @param processId -> The process ID of the target process
 * @return Result -> The result of the operation
 */
Result MacInject::FindProcessId(const char* processName, pid_t& processId)
{
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
    size_t size;

    if (sysctl(mib, 4, nullptr, &size, nullptr, 0) == -1)
    {
        return { false, "Failed to get the size of the process list: " + std::string(strerror(errno)) };
    }

    auto* procList = (struct kinfo_proc*)malloc(size);
    if (procList == nullptr)
    {
        return { false, "Failed to allocate memory for the process list" };
    }

    if (sysctl(mib, 4, procList, &size, nullptr, 0) == -1)
    {
        free(procList);
        return { false, "Failed to get the process list: " + std::string(strerror(errno)) };
    }

    bool found = false;
    for (size_t i = 0; i < size / sizeof(struct kinfo_proc); i++)
    {
        if (strcmp(procList[i].kp_proc.p_comm, processName) == 0)
        {
            processId = procList[i].kp_proc.p_pid;
            found = true;
            break;
        }
    }

    free(procList);

    if (!found)
    {
        return { false, "Failed to find the process: " + std::string(processName) };
    }

    return { true, "Found the process: " + std::string(processName) + " with PID: " + std::to_string(processId) };
}

/**
 * @brief Opens a handle to the target process
 * @param processId -> The process ID of the target process
 * @return Result -> The result of the operation
 */
Result MacInject::OpenProcessHandle(pid_t processId)
{
    kern_return_t kr = task_for_pid(mach_task_self(), processId, &m_targetTask);
    if (kr != KERN_SUCCESS)
    {
        std::string msg = "Failed to get task for PID: " + std::to_string(processId)
                          + " (mach error: " + std::string(mach_error_string(kr))
                          + ", code: " + std::to_string(kr) + ")";
        if (geteuid() != 0)
        {
            msg += " - try running as root";
        }
        else
        {
            msg += " - may be blocked by SIP/entitlements/codesign restrictions";
        }
        return { false, msg };
    }

    return { true, "Opened process handle successfully" };
}

/**
 * @brief Allocates memory in the target process and writes the dylib path to it
 * @return Result -> The result of the operation
 */
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

/**
 * @brief Gets the address of the dlopen function in the target process
 * @return Result -> The result of the operation
 */
Result MacInject::GetDlopenAddress()
{
    m_dlopenAddress = dlsym(RTLD_DEFAULT, "dlopen");
    if (m_dlopenAddress == nullptr)
    {
        return { false, "Failed to get the address of dlopen" };
    }

    return { true, "Got the address of dlopen successfully"  + std::to_string(reinterpret_cast<uintptr_t>(m_dlopenAddress)) };
}

/**
 * @brief Creates a remote thread in the target process to load the dylib
 * @return Result -> The result of the operation
 */
Result MacInject::CreateRemoteThreadToLoadDylib()
{
    thread_act_t thread;
    arm_thread_state64_t state;
    mach_msg_type_number_t stateCount = ARM_THREAD_STATE64_COUNT;

    memset(&state, 0, sizeof(state));
    state.__x[0] = (uint64_t)m_dlopenAddress;
    state.__x[1] = (uint64_t)m_pathAddress;
    state.__pc = (uint64_t)dlopen;

    kern_return_t kr = thread_create_running(m_targetTask, ARM_THREAD_STATE, (thread_state_t)&state, stateCount, &thread);
    if (kr != KERN_SUCCESS)
    {
        mach_vm_deallocate(m_targetTask, m_pathAddress, strlen(m_fullDylibPath) + 1);
        return { false, "Failed to create a remote thread" };
    }

    mach_msg_type_number_t thread_state_count = ARM_THREAD_STATE64_COUNT;
    while (true)
    {
        kr = thread_get_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, &thread_state_count);
        if (kr != KERN_SUCCESS || state.__pc == 0)
        {
            break;
        }
        usleep(100);
    }

    return { true, "Created remote thread successfully and dylib was injected" };
}

/**
 * @brief Prints the results of the injection process
 * @param results -> The results of the injection process
 */
void MacInject::PrintResults(const std::vector<Result>& results)
{
    for (const Result& result : results)
    {
        std::cout << (result.success ? "[SUCCESS] " : "[ERROR] ") << result.message << std::endl;
    }
}

/**
 * @brief Converts a block of memory to a hex string
 * @param data -> The block of memory
 * @param size -> The size of the block of memory
 * @return std::string -> The hex string
 */
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