#include "velvet/support/measurement.h"
#include "support/result.h"

vl_result_t vl_measurement_start_(vl_measurement_t *measurement, const char *name, vl_source_location_t loc) {
    if (!measurement) return VL_ERROR;
    VL_ZERO_OUT(measurement);
    measurement->name = name;
    measurement->tic_loc = loc;
    measurement->tic = clock();
    return VL_SUCCESS;
}

vl_result_t vl_measurement_end_(vl_measurement_t *measurement, vl_source_location_t loc) {
    if (!measurement) return VL_ERROR;
    measurement->toc = clock();
    measurement->toc_loc = loc;
    measurement->diff = (measurement->toc - measurement->tic);
    measurement->seconds = ((float) measurement->diff / (float) CLOCKS_PER_SEC);
    return VL_SUCCESS;
}

vl_result_t vl_measurement_print(vl_measurement_t *measurement) {
    if (!measurement) return VL_ERROR;
    if (!measurement->name) {
        printf("measurement %p ", measurement);
    } else {
        printf("measurement '%s' ", measurement->name);
    }
    if (measurement->tic == 0) {
        printf("has not started yet\n");
        return VL_SUCCESS;
    }
    if (measurement->diff == 0) {
        printf("has not ended yet\n");
    } else {
        printf("took %f seconds (%f milliseconds)\n", measurement->seconds, measurement->seconds * 1000);
    }
    printf("\tstarted at %s:%i\n", measurement->tic_loc.file, measurement->tic_loc.line);
    if (measurement->diff != 0)
        printf("\tended at: %s:%i\n", measurement->toc_loc.file, measurement->toc_loc.line);

    return VL_SUCCESS;
}