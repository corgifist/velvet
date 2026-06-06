#ifndef VELVET_API_H
#define VELVET_API_H

#include "platform.h"

#if VL_PLATFORM(WINDOWS)
    #ifdef VELVET_EXPORT_SYMBOLS
        #define VL_API __declspec(dllexport)
    #else
        #define VL_API __declspec(dllimport)
    #endif // VELET_EXPORT_SYMBOLS
#else // unix-like systems
    #ifdef VELVET_EXPORT_SYMBOLS
        #define VL_API __attribute__((visibility("default")))
    #else
        #define VL_API
    #endif // VELVET_EXPORT_SYMBOLS
#endif

#endif // VELVET_API_H