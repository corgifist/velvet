#ifndef VELVET_GRAPHICS_GEOMETRY_H
#define VELVET_GRAPHICS_GEOMETRY_H

#include <cglm/types.h>

struct vl_point {
    float x, y;
};

typedef struct vl_point vl_point_t;

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

#define VL_POINT_IN_RECT(P, R) \
    (((P).x > (R).x1 && (P).x < (R).x2) && ((P).y > (R).y1 && (P).y < (R).y2))

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

struct vl_quad_uv {
    // top-left and top-right
    vl_point_t tl, tr;
    // bottom-left and bottom-right
    vl_point_t bl, br;
};

typedef struct vl_quad_uv vl_quad_uv_t;

#define VL_POINT(X, Y) ((vl_point_t) {.x = ((float) (X)), .y = ((float) (Y))})
#define VL_POINT1(A) VL_POINT(A, A)

#define VL_POINT_OP(P1, P2, OP) \
    ((vl_point_t) {.x = (P1).x OP (P2).x, .y = (P1).y OP (P2).y})

#define VL_POINT_ADD(P1, P2) \
    VL_POINT_OP(P1, P2, +)

#define VL_POINT_SUB(P1, P2) \
    VL_POINT_OP(P1, P2, -)

#define VL_POINT_MUL(P1, P2) \
    VL_POINT_OP(P1, P2, *)

#define VL_POINT_DIV(P1, P2) \
    VL_POINT_OP(P1, P2, /)

#define VL_RECT(P1, P2) \
    ((vl_rect_t) {.p1 = (P1), .p2 = (P2)})
#define VL_RECT_EX(X1, Y1, X2, Y2) \
    ((vl_rect_t) {.x1 = ((float) (X1)), .y1 = ((float) (Y1)), .x2 = ((float) (X2)), .y2 = ((float) (Y2))})

#define VL_QUAD(P1, P2, P3, P4) \
    ((vl_quad_t) {.p1 = (P1), .p2 = (P2), .p3 = (P3), .p4 = (P4)})
#define VL_QUAD_EX(X1, Y1, X2, Y2, X3, Y3, X4, Y4) \
    ((vl_quad_t) {.x1 = ((float) (X1)), .y1 = ((float) (Y1)), .x2 = ((float) (X2)), .y2 = ((float) (Y2)), \
                    .x3 = ((float) (X3)), .y3 = ((float) (Y3)), .x4 = ((float) (X4)), .y4 = ((float) (Y4))})

#define VL_RECT_TO_QUAD(RECT) \
        VL_QUAD(RECT.p1, VL_POINT(RECT.p2.x, RECT.p1.y), RECT.p2, VL_POINT(RECT.p1.x, RECT.p2.y))

#define VL_QUAD_UV(TL, TR, BL, BR) \
    ((vl_quad_uv_t) {.tl = (TL), .tr =(TR), .bl = (BL), .br = (BR)})

#define VL_QUAD_UV1(A) \
        VL_QUAD_UV(VL_POINT1(A), VL_POINT1(A), VL_POINT1(A), VL_POINT1(A))

#define VL_QUAD_UV_DEFAULT \
        VL_QUAD_UV(VL_POINT(0, 0), VL_POINT(1, 0), VL_POINT(0, 1), VL_POINT(1, 1))

#define VL_QUAD_UV_OP(UV1, UV2, OP) \
    VL_QUAD_UV((UV1).tl.x OP (UV2).tl.y, (UV1).tr.x OP (UV2).tr.y, (UV1).bl.x OP (UV2).bl.y, (UV1).br.x OP (UV2).br.y)

#define VL_QUAD_UV_ADD(UV1, UV2) \
        VL_QUAD_UV_OP(UV1, UV2, +)

#define VL_QUAD_UV_SUB(UV1, UV2) \
        VL_QUAD_UV_OP(UV1, UV2, -)

#define VL_QUAD_UV_MUL(UV1, UV2) \
        VL_QUAD_UV_OP(UV1, UV2, *)

#define VL_QUAD_UV_DIV(UV1, UV2) \
        VL_QUAD_UV_OP(UV1, UV2, /)

#endif // VELVET_GRAPHICS_GEOMETRY_H