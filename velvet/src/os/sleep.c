#include "os/sleep.h"

void vl_os_sleep(vl_platform_context_t *context, uint32_t nanoseconds) {
    if (!context || !context->os_sleep) return;
    return context->os_sleep(nanoseconds);
}