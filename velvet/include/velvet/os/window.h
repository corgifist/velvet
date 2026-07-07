#ifndef VELVET_OS_WINDOW
#define VELVET_OS_WINDOW

#include "velvet/support/result.h"

struct vl_os_window {
    const char *title;
};

typedef struct vl_os_window vl_os_window_t;

vl_result_t vl_os_window_init();
vl_os_window_t *vl_os_window_new(const char *title);
vl_result_t vl_os_window_free(vl_os_window_t *window);

#endif // VELVET_OS_WINDOW