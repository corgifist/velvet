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
    #define VELVET_PLATFORM_MAC 1
#endif // TARGET_OS_MAC

#if defined(__gnu_linux__)
    #define VELVET_PLATFORM_GNU_LINUX 1
#endif // defined(__gnu_linux__)

#if defined(_WIN32)
    #define VELVET_PLATFORM_WINDOWS 1
#endif // defined(_WIN32)

#if !defined(VELVET_PLATFORM_MAC)
    #define VELVET_PLATFORM_MAC 0
#endif // !defined(VELVET_PLATFORM_MAC)

#if !defined(VELVET_PLATFORM_GNU_LINUX)
    #define VELVET_PLATFORM_GNU_LINUX 0
#endif // !defined(VELVET_PLATFORM_GNU_LINUX)

#if !defined(VELVET_PLATFORM_WINDOWS)
    #define VELVET_PLATFORM_WINDOWS 0
#endif // !defined(VELVET_PLATFORM_WINDOWS)

#define VELVET_PLATFORM(NAME) \
    VELVET_PLATFORM_ ## NAME

#endif // VELVET_PLATFORM_H