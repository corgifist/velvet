#ifndef VELVET_API_H
#define VELVET_API_H

#include "platform.h"

#if VL_BUILD_SHARED_LIBS
    #if VL_PLATFORM(WINDOWS)
        #ifdef VL_EXPORT_SYMBOLS
            #define VL_API __declspec(dllexport)
        #else
            #define VL_API __declspec(dllimport)
        #endif // VL_EXPORT_SYMBOLS
    #else // unix-like systems
        #ifdef VL_EXPORT_SYMBOLS
            #define VL_API __attribute__((visibility("default")))
        #else
            #define VL_API
        #endif // VL_EXPORT_SYMBOLS
    #endif
#else
    #define VL_API
#endif


#endif // VELVET_API_H