#include "../../include/badSo.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>

#if defined(__linux__)
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>

void CreateSimpleWindow()
{
}

/**
 * @brief This function is responsible for handling the injection process
 */
void HandleInjection()
{
    char exePath[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len == -1)
    {
        std::cerr << "Failed to get executable path." << std::endl;
        return;
    }
    exePath[len] = '\0';

    void* handle = dlopen(exePath, RTLD_LAZY);
    if (!handle)
    {
        std::cerr << "Failed to load the injected library: " << dlerror() << std::endl;
        return;
    }

    void (*initFunc)() = reinterpret_cast<void (*)()>(dlsym(handle, "InjectedLibraryInit"));
    if (!initFunc)
    {
        std::cerr << "Failed to find the initialization function." << std::endl;
        dlclose(handle);
        return;
    }

    (*initFunc)();

    dlclose(handle);
}

/**
 * @brief This is the entry point for the shared object
 */
extern "C" void* __attribute__((constructor)) DllMain()
{
    {
        pid_t pid = fork();

        if (pid == -1)
        {
            std::cerr << "Failed to fork process." << std::endl;
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            execl("/usr/bin/gnome-calculator", "gnome-calculator", NULL);
            exit(EXIT_FAILURE);
        }
        else
        {
            HandleInjection();
        }
    }

    return nullptr;
}
#endif
