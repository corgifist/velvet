#ifndef VELVET_SUPPORT_BASE_MATH_H
#define VELVET_SUPPORT_BASE_MATH_H


#define VL_PI 3.14159265358979323846
#define VL_DEG2RAD 0.01745329251994329577
#define VL_RAD2DEG 57.2957795130823208768

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

#ifndef VL_SQRTF
#define VL_SQRTF(A) (sqrtf((float) (A)))
#endif // VL_SQRTF

#ifndef VL_SQRF
#define VL_SQRF(A) ((float) (A) * (float) (A))
#endif // VL_SQRF

#endif // VELVET_SUPPORT_BASE_MATH_H