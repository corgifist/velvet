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

vl_os_window_t *vl_os_window_universal_new(const char *title, int w, int h) {
    vl_os_window_universal_t *win = VL_NEW(vl_os_window_universal_t);
    if (!win) return NULL;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    GLFWwindow *handle = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!handle) {
        goto fail;
    }
    glfwShowWindow(handle);

    // immediately poll events to prevent half-broken window from appearing
    glfwPollEvents();
    win->base.title = title;
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

vl_result_t vl_os_window_universal_update_io(vl_os_window_t *window) {
    if (!window) return VL_ERROR;
    vl_os_window_universal_t *win = (vl_os_window_universal_t*) window;

    double dcx, dcy;
    glfwGetCursorPos(win->handle, &dcx, &dcy);
    window->io.cursor.x = (int) dcx;
    window->io.cursor.y = (int) dcy;

    window->io.mouse_down[VL_MOUSE_BUTTON_LEFT] = 
        (glfwGetMouseButton(win->handle, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    window->io.mouse_down[VL_MOUSE_BUTTON_RIGHT] =
        (glfwGetMouseButton(win->handle, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
     window->io.mouse_down[VL_MOUSE_BUTTON_MIDDLE] =
        (glfwGetMouseButton(win->handle, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);

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