#ifndef VELVET_OS_SLEEP_H
#define VELVET_OS_SLEEP_H

#include "velvet/common.h"
#include "velvet/platform/context.h"

/**
 * sleep for a given amount of nanoseconds
 *
 * @param context platform context (check context->os_sleep)
 * @param nanoseconds amount of nanoseconds for sleep 
 */
void vl_os_sleep(vl_platform_context_t *context, uint32_t nanoseconds);

#endif // VELVET_OS_SLEEP_H