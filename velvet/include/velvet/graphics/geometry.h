#ifndef VELVET_GRAPHICS_GEOMETRY_H
#define VELVET_GRAPHICS_GEOMETRY_H

struct vl_point {
    float x, y;
};

typedef struct vl_point vl_point_t;

#define VL_POINT_SUB(P1, P2) \
    ((vl_point_t) {.x = (P1).x - (P2).x, .y = (P1).y - (P2).y})

struct vl_rect {
    union {
        struct {
            vl_point_t p1, p2;
        };
        struct {
            float x1, y1;
            float x2, y2;
        };
    };
};

typedef struct vl_rect vl_rect_t;

struct vl_quad {
    union {
        struct {
            vl_point_t p1, p2, p3, p4;
        };
        struct {
            float x1, y1;
            float x2, y2;
            float x3, y3;
            float x4, y4;
        };
    };
};

typedef struct vl_quad vl_quad_t;

#define VL_POINT(X, Y) ((vl_point_t) {.x = ((float) (X)), .y = ((float) (Y))})

#define VL_RECT(P1, P2) \
    ((vl_rect_t) {.p1 = (P1), .p2 = (P2)})
#define VL_RECT_EX(X1, Y1, X2, Y2) \
    ((vl_rect_t) {.x1 = ((float) (X1)), .y1 = ((float) (Y1)), .x2 = ((float) (X2)), .y2 = ((float) (Y2))})

#define VL_QUAD(P1, P2, P3, P4) \
    ((vl_quad_t) {.p1 = (P1), .p2 = (P2), .p3 = (P3), .p4 = (P4)})
#define VL_QUAD_EX(X1, Y1, X2, Y2, X3, Y3, X4, Y4) \
    ((vl_quad_t) {.x1 = ((float) (X1)), .y1 = ((float) (Y1)), .x2 = ((float) (X2)), .y2 = ((float) (Y2)), \
                    .x3 = ((float) (X3)), .y3 = ((float) (Y3)), .x4 = ((float) (X4)), .y4 = ((float) (Y4))})

#endif // VELVET_GRAPHICS_GEOMETRY_H