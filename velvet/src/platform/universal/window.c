#include "platform/universal/window.h"
#include "os/window.h"
#include "support/memory.h"
#include "support/result.h"

#include <GLFW/glfw3.h>
#include <stdio.h>

vl_result_t vl_os_window_universal_init() {
    int result = glfwInit();
    if (result != GLFW_TRUE) {
        const char *desc;
        glfwGetError(&desc);
        printf("%s\n", desc);
        return VL_ERROR;
    }
    return VL_SUCCESS;
}

vl_os_window_t *vl_os_window_universal_new(const char *title) {
    vl_os_window_universal_t *win = VL_NEW(vl_os_window_universal_t);
    if (!win) return NULL;
    win->base.title = title;
    GLFWwindow *handle = glfwCreateWindow(640, 480, title, NULL, NULL);
    if (!handle) {
        goto fail;
    }
    glfwShowWindow(handle);

    // immediatly polling events to prevent half-broken windows from appearing
    glfwPollEvents();
    win->handle = handle;

    return (vl_os_window_t*) win;

    fail:
    vl_free(win);
    return NULL;
}

vl_result_t vl_os_window_universal_poll_events() {
    glfwPollEvents();
    return VL_SUCCESS;
}

vl_result_t vl_os_window_universal_should_close(vl_os_window_t *window, bool *should_close) {
    if (!window || !should_close) return VL_ERROR;
    vl_os_window_universal_t *win = (vl_os_window_universal_t*) window;
    *should_close = glfwWindowShouldClose(win->handle);
    return VL_SUCCESS;
}

vl_result_t vl_os_window_universal_free(vl_os_window_t *window) {
    if (!window) return VL_ERROR;
    vl_os_window_universal_t *win = (vl_os_window_universal_t*) window;
    glfwDestroyWindow(win->handle);
    vl_free(window);
    return VL_SUCCESS;
}