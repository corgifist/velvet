#ifndef VELVET_SUPPORT_COLOR_H
#define VELVET_SUPPORT_COLOR_H

#include "velvet/support/math.h"
#include "velvet/support/variadic.h"

typedef vl_vec4_t vl_color_t;
typedef vl_vec3_t vl_rgb_t;
typedef vl_vec3_t vl_hsv_t;

#define VL_COLOR4 VL_VEC44
#define VL_COLOR1 VL_VEC41
#define VL_COLOR VL_VEC4

#define VL_ALPHA1(ALPHA) \
    VL_COLOR4(1, 1, 1, ALPHA)
#define VL_ALPHA2(COLOR, ALPHA) \
    VL_COLOR4((COLOR).r, (COLOR).g, (COLOR).b, ALPHA)
#define VL_ALPHA4(R, G, B, A) \
    VL_COLOR4(R, G, B, A)
#define VL_ALPHA(...) \
    VL_VA_DISPATCH(VL_ALPHA, __VA_ARGS__)

#define VL_COLOR_OP(A, B, OP) \
    VL_COLOR((A).r OP (B).r, (A).g OP (B).g, (A).b OP (B).b, (A).a OP (B).a)

#define VL_WHITE VL_COLOR(1, 1, 1, 1)
#define VL_RED VL_COLOR(1, 0, 0, 1)
#define VL_GREEN VL_COLOR(0, 1, 0, 1)
#define VL_BLUE VL_COLOR(0, 0, 1, 1)
#define VL_BLACK VL_COLOR(0, 0, 0, 1)

struct vl_quad_colors {
    // top-left and top-right
    vl_color_t tl, tr;
    // bottom-left and bottom-right
    vl_color_t bl, br;
};

typedef struct vl_quad_colors vl_quad_colors_t;

#define VL_QUAD_COLORS(TL, TR, BL, BR) \
    ((vl_quad_colors_t) {.tl = (TL), .tr = (TR), .bl = (BL), .br = (BR)})

#define VL_QUAD_COLOR(COLOR) \
    VL_QUAD_COLORS(COLOR, COLOR, COLOR, COLOR)

#define VL_QUAD_WHITE \
    VL_QUAD_COLOR(VL_WHITE)

#define VL_QUAD_BLACK \
    VL_QUAD_COLOR(VL_BLACK)

#define VL_QUAD_RED \
    VL_QUAD_COLOR(VL_RED)

#define VL_QUAD_GREEN \
    VL_QUAD_COLOR(VL_GREEN)

#define VL_QUAD_BLUE \
    VL_QUAD_COLOR(VL_BLUE)

struct vl_gradient_stop {
    float percentage;
    vl_color_t color;
};

typedef struct vl_gradient_stop vl_gradient_stop_t;

#define VL_GRADIENT_STOP(P, COLOR) \
    ((vl_gradient_stop_t) {.percentage = (float) (P), .color = (COLOR)})
#define VL_GRADIENT_STOP_EX(P, R, G, B, A) \
    ((vl_gradient_stop_t) {.percentage = (float) (P), .color = VL_COLOR(R, G, B, A)})

VL_API vl_hsv_t vl_rgb_to_hsv(vl_rgb_t rgb);
VL_API vl_rgb_t vl_hsv_to_rgb(vl_hsv_t hsv);
VL_API vl_color_t vl_color_hue_shift(vl_color_t color, float hue);

#endif // VELVET_GRAPHICS_COLOR_H