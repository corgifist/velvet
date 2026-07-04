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

#define VL_VA_NARGS_(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, COUNT, ...) COUNT
/**
 * Keep in mind that 16 is the limit of arguments for VL_VA_NARGS
 * Passing more that 16 arguments may yield unexpected and unwanted results
 */
#define VL_VA_NARGS(...) VL_VA_NARGS_(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#endif // VELVET_API_H