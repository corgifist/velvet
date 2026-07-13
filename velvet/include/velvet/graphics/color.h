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

#define VL_QUAD_WHITE \
    VL_QUAD_COLORS(VL_WHITE, VL_WHITE, VL_WHITE, VL_WHITE)

#endif // VELVET_GRAPHICS_COLOR_H