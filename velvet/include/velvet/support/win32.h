#ifndef VELVET_SUPPORT_WIN32_H
#define VELVET_SUPPORT_WIN32_H

#include "velvet/support/platform.h"

#if VL_PLATFORM(WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <shlobj.h>
#endif

#endif // VELVET_SUPPORT_WIN32_H