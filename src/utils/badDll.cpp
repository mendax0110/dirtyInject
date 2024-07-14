#include "../../include/badDll.h"

#include <iostream>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <codecvt>
#include <locale>

/**
 * @brief The window procedure for the injected window
 * @param hwnd -> The handle to the window
 * @param uMsg -> The message
 * @param wParam -> The message parameter
 * @param lParam -> The message parameter
 * @return LRESULT -> The result of the message processing
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

/**
 * @brief Create a Simple Window object
 */
void CreateSimpleWindow()
{
    const char CLASS_NAME[] = "InjectedWindowClass";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, "Failed to register window class.", "Error", MB_ICONERROR | MB_OK);
        return;
    }

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "DLL Injected Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Failed to create window.", "Error", MB_ICONERROR | MB_OK);
        return;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

/**
 * @brief The thread function for the window
 * @param lpParam -> The thread parameters
 * @return DWORD -> The thread return value
 */
DWORD WINAPI WindowThread(LPVOID lpParam)
{
    CreateSimpleWindow();
    return 0;
}

/**
 * @brief The entry point for the DLL
 * @param hModule -> The handle to the DLL module
 * @param ul_reason_for_call -> reason for calling the DLL
 * @param lpReserved -> reserved parameter
 * @return BOOL -> The result of the operation
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        if (CreateThread(NULL, 0, WindowThread, NULL, 0, NULL) == NULL)
        {
            MessageBox(NULL, "Failed to create thread.", "Error", MB_ICONERROR | MB_OK);
        }

        {
            STARTUPINFO si = {};
            PROCESS_INFORMATION pi = {};
            si.cb = sizeof(si);

            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            
            
            for (int i = 0; i < 10; i++)
            {
                std::string narrowCommandLine = converter.to_bytes(L"calc.exe");
                CreateProcess(NULL, const_cast<char*>(narrowCommandLine.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

                if (!CreateProcess(NULL, const_cast<char*>(narrowCommandLine.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
                {
                    MessageBox(NULL, "Failed to start Calculator.", "Error", MB_ICONERROR | MB_OK);
                }
                else
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif