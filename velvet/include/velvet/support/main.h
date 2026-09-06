#ifndef VELVET_SUPPORT_MAIN_H
#define VELVET_SUPPORT_MAIN_H

#include "velvet/support/platform.h"
#include "velvet/support/win32.h"
#include <locale.h>

#if VL_PLATFORM(WINDOWS)
    #define main(...) \
        __vl_main(int argc, const char *argv[]); \
        int main(int argc, const char *argv[]) { \
            SetConsoleOutputCP(CP_UTF8); \
            setlocale(LC_ALL, ".UTF8"); \
            return __vl_main(argc, argv); \
        } \
        int __vl_main(int argc, const char *argv[])
#else
    #define main(...) main(__VA_ARGS__)
#endif

#endif // VELVET_SUPPORT_MAIN_H