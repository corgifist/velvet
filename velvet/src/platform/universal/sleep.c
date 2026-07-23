#include "platform/universal/sleep.h"
#include "support/platform.h"

#if VL_PLATFORM(WINDOWS)
    // for Sleep()
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    // for usleep()
    #include <unistd.h>
#endif // VL_PLATFORM(WINDOWS)

void vl_os_sleep_universal(uint32_t nanoseconds) {
#if VL_PLATFORM(WINDOWS)
    Sleep(nanoseconds);
#else
    usleep(nanoseconds * 1000);
#endif
}