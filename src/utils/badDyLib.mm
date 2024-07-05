#include <iostream>
#include <vector>

#if defined(__APPLE__)
#include <ApplicationServices/ApplicationServices.h>
#include <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/badDyLib.h"


void* CreateSimpleWindow(void* arg)
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

void LaunchCalculator()
{
    //system(open -a Calculator);
}

__attribute__((constructor))
void DllMain()
{
    pthread_t thread;
    if (pthread_create(&thread, nullptr, CreateSimpleWindow, nullptr) != 0)
    {
        std::cerr << "Failed to create thread" << std::endl;
    }
    else
    {
        pthread_detach(thread);
    }

    LaunchCalculator();
}
#endif