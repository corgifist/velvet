#ifndef VELVET_SUPPORT_MEASUREMENT_H
#define VELVET_SUPPORT_MEASUREMENT_H

#include "velvet/support/result.h"
#include "velvet/support/api.h"
#include "velvet/support/memory.h"
#include <time.h>

struct vl_measurement {
    // time in clock - tic
    // time out clock - toc
    clock_t tic, toc;
    clock_t diff;
    float seconds;
    const char *name;
    vl_source_location_t tic_loc, toc_loc;
};
typedef struct vl_measurement vl_measurement_t;

VL_API vl_result_t vl_measurement_start_(vl_measurement_t *measurement, const char *name, vl_source_location_t loc);
#define vl_measurement_start(measurement, name) \
    (vl_measurement_start_(measurement, name, VL_HERE))

VL_API vl_result_t vl_measurement_end_(vl_measurement_t *measurement, vl_source_location_t loc);
#define vl_measurement_end(measurement) \
    (vl_measurement_end_(measurement, VL_HERE))

VL_API vl_result_t vl_measurement_print(vl_measurement_t *measurement);



#endif // VELVET_SUPPORT_MEASUREMENT