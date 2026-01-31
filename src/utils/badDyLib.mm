#include <iostream>
#include <vector>

#if defined(__APPLE__)
#include <ApplicationServices/ApplicationServices.h>
#include <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/badDyLib.h"

/**
 * @brief Create a simple window using Cocoa
 * @param arg -> void*
 * @return void* -> nullptr
 */
void* CreateSimpleWindow()
{
    @autoreleasepool
    {
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        NSRect frame = NSMakeRect(0, 0, 500, 300);
        NSWindow *window = [[NSWindow alloc] initWithContentRect:frame
                                                       styleMask:(NSWindowStyleMaskTitled |
                                                                  NSWindowStyleMaskClosable |
                                                                  NSWindowStyleMaskResizable)
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        [window setTitle:@"Dll INjected Window"];
        [window makeKeyAndOrderFront:nil];
        [app activateIgnoringOtherApps:YES];
        [app run];
    }

    return nullptr;
}

/**
 * @brief Launch the calculator application
 */
void LaunchCalculator()
{
    system("open -a Calculator");
}

/**
 * @brief Entry point for the dynamic library
 */
__attribute__((constructor))
void DllMain()
{
    /*pthread_t thread;
    if (pthread_create(&thread, nullptr, CreateSimpleWindow, nullptr) != 0)
    {
        std::cerr << "Failed to create thread" << std::endl;
    }
    else
    {
        pthread_detach(thread);
    }*/

    dispatch_async(dispatch_get_main_queue(), ^
    {
        CreateSimpleWindow();
    });

    LaunchCalculator();
}
#endif