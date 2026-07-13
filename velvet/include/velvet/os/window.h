#ifndef VELVET_OS_WINDOW
#define VELVET_OS_WINDOW

#include "velvet/common.h"
#include "velvet/support/result.h"

struct vl_os_window {
    const char *title;
};

typedef struct vl_os_window vl_os_window_t;

vl_result_t vl_os_window_init();

vl_os_window_t *vl_os_window_new(const char *title, int w, int h);
vl_result_t vl_os_window_should_close(vl_os_window_t *window, bool *should_close);
vl_result_t vl_os_window_poll_events();
vl_result_t vl_os_window_free(vl_os_window_t *window);

#endif // VELVET_OS_WINDOW