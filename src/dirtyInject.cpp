#include "../include/winInject.h"
#include "../include/linInject.h"
#include "../include/macInject.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <DLL/So/DyLib Name> <Process Name>" << std::endl;
        return 1;
    }

#if defined(_WIN32) || defined(_WIN64)
    const char* dllName = argv[1];
    const char* processName = argv[2];
    WinInject injector(dllName, processName);
    std::vector<Result> results = injector.InjectDll();
#endif

#if defined(__linux__)
    const char* soName = argv[1];
    const char* processName = argv[2];
    LinInject injector(soName, processName);
    std::vector<Result> results = injector.InjectSo();
#endif

#if defined(__APPLE__)
    const char* dylibName = argv[1];
    const char* processName = argv[2];
    MacInject injector(dylibName, processName);
    std::vector<Result> results = injector.InjectDylib();
#endif

    if (!results.empty() && !results.back().success)
    {
        std::cerr << "Failed to inject the DLL" << std::endl;
        for (const auto& result : results)
        {
            std::cerr << "Error: " << result.message << std::endl;
        }
        return 1;
    }

    injector.PrintResults(results);

    std::cout << "DLL injected successfully" << std::endl;
    return 0;
}