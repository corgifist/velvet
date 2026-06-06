/*
    platform.h - platform detection utils
*/

#ifndef VELVET_PLATFORM_H
#define VELVET_PLATFORM_H

#if defined(__APPLE__)
    // TARGET_OS_MAC
    #include <TargetConditionals.h>
#endif // defined(__APPLE__)

#if TARGET_OS_MAC == 1
    #define VL_PLATFORM_MAC 1
#endif // TARGET_OS_MAC

#if defined(__gnu_linux__)
    #define VL_PLATFORM_GNU_LINUX 1
#endif // defined(__gnu_linux__)

#if defined(_WIN32)
    #define VL_PLATFORM_WINDOWS 1
#endif // defined(_WIN32)

#if !defined(VL_PLATFORM_MAC)
    #define VL_PLATFORM_MAC 0
#endif // !defined(VL_PLATFORM_MAC)

#if !defined(VL_PLATFORM_GNU_LINUX)
    #define VL_PLATFORM_GNU_LINUX 0
#endif // !defined(VL_PLATFORM_GNU_LINUX)

#if !defined(VL_PLATFORM_WINDOWS)
    #define VL_PLATFORM_WINDOWS 0
#endif // !defined(VL_PLATFORM_WINDOWS)

/*
    available platforms: WINDOWS, MAC, GNU_LINUX

    usage:
        #if VL_PLATFORM(WINDOWS)
            ...
        #elif VL_PLATFORM(MAC)
        #endif
*/
#define VL_PLATFORM(NAME) \
    VL_PLATFORM_ ## NAME

#endif // VELVET_PLATFORM_H