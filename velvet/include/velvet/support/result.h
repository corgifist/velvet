#ifndef VELVET_RESULT_H
#define VELVET_RESULT_H

enum vl_result {
    VL_RESULT_FORCE_SIGNED = -2147483647,
    VL_SUCCESS = 0,
    VL_ERROR = 1,
    VL_RESULT_FORCE_INT = 2147483647
};

typedef enum vl_result vl_result_t;

#endif // VELVET_RESULT_H