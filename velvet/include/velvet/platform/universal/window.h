/*
    platform/universal/window.h - implementation of vl_os_window using GLFW
*/

#ifndef VELVET_PLATFORM_UNIVERSAL_WINDOW_H
#define VELVET_PLATFORM_UNIVERSAL_WINDOW_H

#include "velvet/os/window.h"

#include <GLFW/glfw3.h>

struct vl_os_window_universal {
    vl_os_window_t base;

    GLFWwindow *handle;
};

typedef struct vl_os_window_universal vl_os_window_universal_t;

vl_result_t vl_os_window_universal_init();
vl_os_window_t *vl_os_window_universal_new(const char *title, int w, int h);
vl_result_t vl_os_window_universal_should_close(vl_os_window_t *window, bool *should_close);
vl_result_t vl_os_window_universal_poll_events();
vl_result_t vl_os_window_universal_free(vl_os_window_t *window);

#endif // VELVET_PLATFORM_UNIVERSAL_WINDOW_H