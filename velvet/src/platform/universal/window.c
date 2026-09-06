#include "platform/universal/window.h"
#include "graphics/render.h"
#include "os/window.h"
#include "support/allocator.h"
#include "support/da.h"
#include "support/memory.h"
#include "support/result.h"

#include <GLFW/glfw3.h>
#include <stdio.h>

static bool s_initialized = false;

vl_result_t vl_os_window_universal_init() {
    int result = glfwInit();
    if (result != GLFW_TRUE) {
        const char *desc;
        glfwGetError(&desc);
        printf("%s\n", desc);
        return VL_ERROR;
    }
    s_initialized = true;
    return VL_SUCCESS;
}

typedef struct {
    vl_os_window_t *window;
    GLFWwindow *handle;
} vl_window_handle_pair_t;

static VL_DA(vl_os_window_t*) s_windows = NULL;
static VL_DA(vl_window_handle_pair_t) s_pairs = NULL;

static void callback_window_resize(GLFWwindow *window, int w, int h) {
    if (!s_pairs) return;
    for (int i = 0; i < VL_DA_LENGTH(s_pairs); i++) {
        vl_window_handle_pair_t *pair = s_pairs + i;
        if (pair->handle == window && pair->window->callback_resize) {
            pair->window->callback_resize(pair->window, w, h);
        }
    }
}

vl_os_window_t *vl_os_window_universal_new(const char *title, int w, int h) {
    if (!s_initialized) {
        if (vl_os_window_universal_init()) return NULL;
    }
    vl_os_window_universal_t *win = VL_NEW(vl_os_window_universal_t);
    if (!win) return NULL;
    if (!s_pairs) {
        s_pairs = VL_DA_INIT_WITH_ALLOCATOR(vl_window_handle_pair_t, VL_ALLOCATOR_STDLIB());
        s_windows = VL_DA_INIT_WITH_ALLOCATOR(vl_os_window_t*, VL_ALLOCATOR_STDLIB());
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    GLFWwindow *handle = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!handle) {
        goto fail;
    }
    glfwShowWindow(handle);

    // immediately poll events to prevent half-broken window from appearing
    glfwPollEvents();

    win->base.title = title;
    win->base.owned_renders = VL_DA_INIT(vl_graphics_render_t*);
    win->base.io = (vl_os_window_io_t) {0};
    win->handle = handle;

    *VL_DA_PUSH(s_pairs, vl_window_handle_pair_t) = (vl_window_handle_pair_t) {
        (vl_os_window_t*) win, handle
    };
    *VL_DA_PUSH(s_windows, vl_os_window_t*) = (vl_os_window_t*) win;

    glfwSetWindowSizeCallback(handle, callback_window_resize);
    vl_os_window_universal_poll_events();

    return (vl_os_window_t*) win;

    fail:
    vl_free(win);
    return NULL;
}

static void update_window_io(vl_os_window_t *window) {
    if (!window) return;
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

    float scale_x, scale_y;
    glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &scale_x, &scale_y);
    window->io.content_scale = (scale_x + scale_y) / 2.0f;

    int window_width, window_height;
    glfwGetWindowSize(win->handle, &window_width, &window_height);
    window->io.window_size = VL_VEC2(window_width, window_height);
}

vl_result_t vl_os_window_universal_poll_events() {
    if (!s_windows) return VL_ERROR;
    glfwPollEvents();
    for (int i = 0; i < VL_DA_LENGTH(s_windows); i++) {
        if (s_windows[i]) {
            update_window_io(s_windows[i]);
        }
    }
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
    for (int i = 0; i < VL_DA_LENGTH(s_windows); i++) {
        if (s_windows[i] == window) {
            s_windows[i] = NULL;
        }
    }
    VL_DA_FREE(win->base.owned_renders);
    vl_free(window);
    return VL_SUCCESS;
}