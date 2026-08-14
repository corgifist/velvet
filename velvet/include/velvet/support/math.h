#ifndef VELVET_SUPPORT_MATH_H
#define VELVET_SUPPORT_MATH_H

#include <math.h>

#define VL_MIN(A, B) \
    ((A) > (B) ? (B) : (A))

#define VL_MAX(A, B) \
    ((A) > (B) ? (A) : (B))

#ifndef VL_CEIL
#define VL_CEIL(A) (ceilf((float) (A)))
#endif // VL_CEIL

#ifndef VL_FLOOR
#define VL_FLOOR(A) (floorf((float) (A)))
#endif // VL_FLOOR

#ifndef VL_ROUND
#define VL_ROUND(A) (roundf((float) (A)))
#endif // VL_ROUND

#ifndef VL_ICEIL
#define VL_ICEIL(A) ((int) VL_CEIL(A))
#endif // VL_ICEIL

#ifndef VL_IFLOOR
#define VL_IFLOOR(A) ((int) VL_FLOOR(A))
#endif // VL_IFLOOR

#ifndef VL_IROUND
#define VL_IROUND(A) ((int) VL_ROUND(A))
#endif // VL_IROUND

#endif // VELVET_SUPPORT_MATH_H