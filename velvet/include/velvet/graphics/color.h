#ifndef VELVET_GRAPHICS_COLOR_H
#define VELVET_GRAPHICS_COLOR_H

struct vl_color {
    float r, g, b, a;
};

typedef struct vl_color vl_color_t;

#define VL_COLOR(R, G, B, A) \
    ((vl_color_t) {.r = ((float) (R)), .g = ((float) (G)), .b = ((float) (B)), .a = ((float) (A))})

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

#endif // VELVET_GRAPHICS_COLOR_H