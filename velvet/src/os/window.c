#include "velvet/os/window.h"
#include "support/result.h"
#include "velvet/support/feature.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#if VL_FEATURE(UNIVERSAL_PLATFORM)
    #include "velvet/platform/universal/window.h"
#endif // FEATURE(UNIVERSAL_PLATFORM)

static bool s_initialized = false;

vl_result_t vl_os_window_init() {
    if (s_initialized) return VL_SUCCESS;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    vl_result_t result = vl_os_window_universal_init();
    if (!result) s_initialized = true;
    return result;
#endif
    printf("no platform for vl_os_window\n");
    return VL_ERROR;
}

vl_os_window_t *vl_os_window_new(const char *input, int w, int h) {
    if (!s_initialized) return NULL;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return (vl_os_window_t*) vl_os_window_universal_new(input, w, h);
#endif
    printf("no platform for vl_os_window\n");
    return NULL;
}

vl_result_t vl_os_window_poll_events() {
    if (!s_initialized) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_os_window_universal_poll_events();
#endif
    printf("no platform for vl_os_window\n");
    return VL_ERROR;
}

vl_result_t vl_os_window_should_close(vl_os_window_t *window, bool *should_close) {
    if (!s_initialized) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_os_window_universal_should_close(window, should_close);
#endif
    return VL_SUCCESS;
}

vl_result_t vl_os_window_free(vl_os_window_t *window) {
    if (!s_initialized) return VL_ERROR;
#if VL_FEATURE(UNIVERSAL_PLATFORM)
    return vl_os_window_universal_free(window);
#endif
    printf("no platform for vl_os_window\n");
    return VL_ERROR;
}