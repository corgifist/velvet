#ifndef VELVET_GRAPHICS_COLOR_H
#define VELVET_GRAPHICS_COLOR_H

struct vl_color {
    float r, g, b, a;
};

typedef struct vl_color vl_color_t;

#define VL_COLOR(R, G, B, A) \
    ((vl_color_t) {.r = ((float) (R)), .g = ((float) (G)), .b = ((float) (B)), .a = ((float) (A))})

#endif // VELVET_GRAPHICS_COLOR_H