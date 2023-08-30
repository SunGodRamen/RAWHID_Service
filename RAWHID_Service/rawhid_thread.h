#ifndef RAWHID_THREAD_H
#define RAWHID_THREAD_H

#include <windows.h>
#include <stdbool.h>
#include <string.h>
#include "rawhid.h"
#include "interthread_comm.h"

DWORD WINAPI rawhid_device_thread(LPVOID device_info);

#endif