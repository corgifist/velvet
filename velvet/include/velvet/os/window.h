#ifndef VELVET_OS_WINDOW
#define VELVET_OS_WINDOW

#include "velvet/common.h"
#include "velvet/support/da.h"
#include "velvet/support/result.h"
#include "velvet/graphics/geometry.h"

enum vl_os_mouse_button {
    VL_MOUSE_BUTTON_LEFT = 0,
    VL_MOUSE_BUTTON_RIGHT,
    VL_MOUSE_BUTTON_MIDDLE,
    VL_MOUSE_BUTTON_COUNT
};

typedef enum vl_os_mouse_button vl_os_mouse_button_t;

struct vl_os_window_io {
    // cursor x/y are relative to the window itself
    vl_point_t cursor;
    bool mouse_down[VL_MOUSE_BUTTON_COUNT];
};

typedef struct vl_os_window_io vl_os_window_io_t;

struct vl_os_window;
typedef void (*vl_os_window_callback_resize_t)(struct vl_os_window *window, int w, int h);

struct vl_graphics_render;
struct vl_os_window {
    const char *title;
    vl_os_window_io_t io;
    VL_DA(struct vl_graphics_render*) owned_renders;

    vl_os_window_callback_resize_t callback_resize;
};

typedef struct vl_os_window vl_os_window_t;

vl_result_t vl_os_window_init();

vl_os_window_t *vl_os_window_new(const char *title, int w, int h);
vl_result_t vl_os_window_should_close(vl_os_window_t *window, bool *should_close);
vl_result_t vl_os_window_poll_events();
vl_result_t vl_os_window_free(vl_os_window_t *window);

#endif // VELVET_OS_WINDOW