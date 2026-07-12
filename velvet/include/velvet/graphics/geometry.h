#ifndef VELVET_GRAPHICS_GEOMETRY_H
#define VELVET_GRAPHICS_GEOMETRY_H

struct vl_point {
    float x, y;
};

typedef struct vl_point vl_point_t;

#define VL_POINT(X, Y) ((vl_point_t) {.x = ((float) (X)), .y = ((float) (Y))})

struct vl_rect {
    float x1, y1;
    float x2, y2;
};

typedef struct vl_rect vl_rect_t;

#define VL_RECT(P1, P2) \
    VL_RECT_EX((P1).x, (P1).y, (P2).x, (P2).y)
#define VL_RECT_EX(X1, Y1, X2, Y2) \
    ((vl_rect_t) {.x1 = ((float) (X1)), .y1 = ((float) (Y1)), .x2 = ((float) (X2)), .y2 = ((float) (Y2))})

#endif // VELVET_GRAPHICS_GEOMETRY_H