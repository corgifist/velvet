#ifndef VELVET_OS_SLEEP_H
#define VELVET_OS_SLEEP_H

#include "velvet/common.h"

/**
 * sleep for a given amount of nanoseconds
 *
 * @param nanoseconds amount of nanoseconds for sleep 
 */
void vl_os_sleep(uint32_t nanoseconds);

#endif // VELVET_OS_SLEEP_H