#include "velvet/os/window.h"
#include "platform/context.h"
#include "velvet/support/feature.h"

vl_os_window_t *vl_os_window_new(vl_platform_context_t *context, const char *title, int w, int h) {
    if (!vl_platform_context_valid(context) || !context->os_window_new) return NULL;
    vl_os_window_t *window = context->os_window_new(title, w, h);
    if (window) {
        window->context = context;
    }
    return window;
}

vl_result_t vl_os_window_poll_events(vl_platform_context_t *context) {
    if (!vl_platform_context_valid(context) || !context->os_window_poll_events) return VL_ERROR;
    return context->os_window_poll_events();
}

vl_result_t vl_os_window_should_close(vl_os_window_t *window, bool *should_close) {
    if (!window || !vl_platform_context_valid(window->context) || !window->context->os_window_should_close) return VL_ERROR;
    return window->context->os_window_should_close(window, should_close);
}

vl_result_t vl_os_window_free(vl_os_window_t *window) {
    if (!window || !vl_platform_context_valid(window->context) || !window->context->os_window_free) return VL_ERROR;
    return window->context->os_window_free(window);
}