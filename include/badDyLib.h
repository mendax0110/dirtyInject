#ifndef BADDYLIB_H
#define BADDYLIB_H

#include <iostream>

#if defined(__APPLE__)
#include <pthread.h>
#include <unistd.h>

void* CreateSimpleWindow(void* arg);

void LaunchCalculator();

void DllMain() __attribute__((constructor));

#endif

#endif