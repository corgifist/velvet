#ifndef VELVET_SUPPORT_MATH_H
#define VELVET_SUPPORT_MATH_H

#include "velvet/support/base_math.h"
#include "velvet/support/variadic.h"
#include "velvet/support/api.h"
#include "velvet/support/result.h"

union vl_vec2 {
    struct {
        float x, y;
    };
    float m[2];
};

typedef union vl_vec2 vl_vec2_t;
typedef union vl_vec2 vl_point_t;

#define VL_VEC22(X, Y) \
    ((vl_vec2_t) {.x = (float) (X), .y = (float) (Y)})
#define VL_VEC21(X) \
    VL_VEC22(X, X)
#define VL_VEC2(...) \
    VL_VA_DISPATCH(VL_VEC2, __VA_ARGS__)
#define VL_POINT VL_VEC2
#define VL_VEC2_LEN(VEC2) VL_SQRTF(VL_SQRF(VEC2.x) + VL_SQRF(VEC2.y))
#define VL_VEC2_SCALE(VEC2, SCALE) \
    VL_VEC2((VEC2).x * (SCALE), (VEC2).y * (SCALE))

#define VL_VEC2_OP(A, B, OP) \
    ((vl_vec2_t) {.x = (A).x OP (B).x, .y = (A).y OP (B).y})
#define VL_VEC2_ADD(A, B) \
    VL_VEC2_OP(A, B, +)
#define VL_VEC2_SUB(A, B) \
    VL_VEC2_OP(A, B, -)
#define VL_VEC2_MUL(A, B) \
    VL_VEC2_OP(A, B, *)
#define VL_VEC2_DIV(B) \
    VL_VEC2_OP(A, B, /)

#define VL_POINT_OP VL_VEC2_OP
#define VL_POINT_ADD VL_VEC2_ADD
#define VL_POINT_SUB VL_VEC2_SUB
#define VL_POINT_MUL VL_VEC2_MUL
#define VL_POINT_DIV VL_VEC2_DIV

union vl_vec3 {
    struct {
        float x, y, z;
    };
    struct {
        float r, g, b;
    };
    float m[3];
};
typedef union vl_vec3 vl_vec3_t;

#define VL_VEC33(X, Y, Z) \
    ((vl_vec3_t) {.x = (float) (X), .y = (float) (Y), .z = (float) (Z)})
#define VL_VEC31(A) \
    VL_VEC33(A, A, A)
#define VL_VEC3(...) \
    VL_VA_DISPATCH(VL_VEC3, __VA_ARGS__)
#define VL_VEC3_LEN(VEC3) VL_SQRTF(VL_SQRF(VEC3.x) + VL_SQRF(VEC3.y) + VL_SQRF(VEC3.z))
#define VL_VEC3_SCALE(VEC3, SCALE) \
    VL_VEC3((VEC3).x * (SCALE), (VEC3).y * (SCALE), (VEC3).z * (SCALE))

#define VL_VEC3_OP(A, B, OP) \
    ((vl_vec3_t) {.x = (A).x OP (B).x, .y = (A).y OP (B).y, .z = (A).z OP (B).z})
#define VL_VEC3_ADD(A, B) \
    VL_VEC3_OP(A, B, +)
#define VL_VEC3_SUB(A, B) \
    VL_VEC3_OP(A, B, -)
#define VL_VEC3_MUL(A, B) \
    VL_VEC3_OP(A, B, *)
#define VL_VEC3_DIV(B) \
    VL_VEC3_OP(A, B, /)

union vl_vec4 {
    struct {
        float x, y, z, w;
    };
    struct {
        float r, g, b, a;
    };
    float m[4];
};
typedef union vl_vec4 vl_vec4_t;

#define VL_VEC44(X, Y, Z, W) \
    ((vl_vec4_t) {.x = (float) (X), .y = (float) (Y), .z = (float) (Z), .w = (float) (W)})
#define VL_VEC41(A) \
    VL_VEC44(A, A, A, A)
#define VL_VEC4(...) \
    VL_VA_DISPATCH(VL_VEC4, __VA_ARGS__)
#define VL_VEC4_LEN(VEC4) VL_SQRTF(VL_SQRF(VEC4.x) + VL_SQRF(VEC4.y) + VL_SQRF(VEC4.z) + VL_SQRF(VEC4.w))
#define VL_VEC4_SCALE(VEC4, SCALE) \
    VL_VEC4((VEC4).x * (SCALE), (VEC4).y * (SCALE), (VEC4).z * (SCALE), (VEC4).w * (SCALE))

#define VL_VEC4_OP(A, B, OP) \
    ((vl_vec4_t) {.x = (A).x OP (B).x, .y = (A).y OP (B).y, .z = (A).z OP (B).z, .w = (A).w OP (B).w})
#define VL_VEC4_ADD(A, B) \
    VL_VEC4_OP(A, B, +)
#define VL_VEC4_SUB(A, B) \
    VL_VEC4_OP(A, B, -)
#define VL_VEC4_MUL(A, B) \
    VL_VEC4_OP(A, B, *)
#define VL_VEC4_DIV(B) \
    VL_VEC4_OP(A, B, /)

union vl_ivec4 {
    struct {
        int x, y, z, w;
    };
    struct {
        int r, g, b, a;
    };
    int m[4];
};
typedef union vl_ivec4 vl_ivec4_t;

#define VL_IVEC44(X, Y, Z, W) \
    ((vl_ivec4_t) {.x = (int) (X), .y = (int) (Y), .z = (int) (Z), .w = (int) (W)})
#define VL_IVEC41(A) \
    VL_IVEC44(A, A, A, A)
#define VL_IVEC4(...) \
    VL_VA_DISPATCH(VL_IVEC4, __VA_ARGS__)
#define VL_IVEC4_LEN(VEC4) VL_SQRTF(VL_SQRF(VEC4.x) + VL_SQRF(VEC4.y) + VL_SQRF(VEC4.z) + VL_SQRF(VEC4.w))
#define VL_IVEC4_SCALE(VEC4, SCALE) \
    VL_IVEC4((VEC4).x * (SCALE), (VEC4).y * (SCALE), (VEC4).z * (SCALE), (VEC4).w * (SCALE))

#define VL_IVEC4_OP(A, B, OP) \
    ((vl_ivec4_t) {.x = (A).x OP (B).x, .y = (A).y OP (B).y, .z = (A).z OP (B).z, .w = (A).w OP (B).w})
#define VL_IVEC4_ADD(A, B) \
    VL_IVEC4_OP(A, B, +)
#define VL_IVEC4_SUB(A, B) \
    VL_IVEC4_OP(A, B, -)
#define VL_IVEC4_MUL(A, B) \
    VL_IVEC4_OP(A, B, *)
#define VL_IVEC4_DIV(B) \
    VL_IVEC4_OP(A, B, /)

union vl_rect {
    struct {
        vl_point_t p1, p2;
    };
    struct {
        float x1, y1;
        float x2, y2;
    };
};
typedef union vl_rect vl_rect_t;

#define VL_RECT(P1, P2) \
    ((vl_rect_t) {.p1 = (P1), .p2 = (P2)})
#define VL_RECT_EX(X1, Y1, X2, Y2) \
    ((vl_rect_t) {.x1 = ((float) (X1)), .y1 = ((float) (Y1)), .x2 = ((float) (X2)), .y2 = ((float) (Y2))})

#define VL_POINT_IN_RECT(P, R) \
    (((P).x > (R).x1 && (P).x < (R).x2) && ((P).y > (R).y1 && (P).y < (R).y2))

struct vl_line {
    vl_point_t from;
    vl_point_t to;
    float thickness;
};

typedef struct vl_line vl_line_t;

#define VL_LINE(FROM, TO, THICKNESS) \
    ((vl_line_t) {.from = (vl_point_t) (FROM), .to = (vl_point_t) (TO), .thickness = (float) (THICKNESS)})

union vl_quad {
    struct {
        vl_point_t p1;
        vl_point_t p2;
        vl_point_t p3;
        vl_point_t p4;
    };
    struct {
        float x1, y1;
        float x2, y2;
        float x3, y3;
        float x4, y4;
    };
};
typedef union vl_quad vl_quad_t;

#define VL_RECT_TO_QUAD(RECT) \
    VL_QUAD(RECT.p1, VL_POINT(RECT.p2.x, RECT.p1.y), RECT.p2, VL_POINT(RECT.p1.x, RECT.p2.y))
#define VL_QUAD(P1, P2, P3, P4) \
    ((vl_quad_t) {.p1 = (P1), .p2 = (P2), .p3 = (P3), .p4 = (P4)})
#define VL_QUAD_EX(X1, Y1, X2, Y2, X3, Y3, X4, Y4) \
    ((vl_quad_t) {.x1 = ((float) (X1)), .y1 = ((float) (Y1)), .x2 = ((float) (X2)), .y2 = ((float) (Y2)), \
                    .x3 = ((float) (X3)), .y3 = ((float) (Y3)), .x4 = ((float) (X4)), .y4 = ((float) (Y4))})

struct vl_quad_uv {
    // top-left and top-right
    vl_point_t tl, tr;
    // bottom-left and bottom-right
    vl_point_t bl, br;
};
typedef struct vl_quad_uv vl_quad_uv_t;

#define VL_QUAD_UV(TL, TR, BL, BR) \
    ((vl_quad_uv_t) {.tl = (TL), .tr =(TR), .bl = (BL), .br = (BR)})
#define VL_QUAD_UV1(A) \
        VL_QUAD_UV(VL_POINT(A), VL_POINT(A), VL_POINT(A), VL_POINT(A))
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

union vl_mat4 {
    struct {
        vl_vec4_t m1, m2, m3, m4;
    };
    vl_vec4_t m[4];
};
typedef union vl_mat4 vl_mat4_t;

#define VL_MAT44(A, B, C, D) \
    ((vl_mat4_t) {.m1 = A, .m2 = B, .m3 = C, .m4 = D})
#define VL_MAT40() \
    ((vl_mat4_t) {.m1 = {1, 0, 0, 0}, .m2 = {0, 1, 0, 0}, .m3 = {0, 0, 1, 0}, .m4 = {0, 0, 0, 1}})
#define VL_MAT4(...) \
    VL_VA_DISPATCH(VL_MAT4, __VA_ARGS__)

// Matrix logic is based on linmath.h
// Check it out here: https://github.com/datenwolf/linmath.h 

VL_API vl_result_t vl_mat4_dup(vl_mat4_t *dst, const vl_mat4_t src);
VL_API vl_result_t vl_mat4_mul(vl_mat4_t *dst, const vl_mat4_t a, const vl_mat4_t b);
VL_API vl_result_t vl_mat4_mul_vec4(vl_vec4_t *dst, const vl_mat4_t a, const vl_vec4_t b);
VL_API vl_result_t vl_mat4_add(vl_mat4_t *dst, const vl_mat4_t a, const  vl_mat4_t b);
VL_API vl_result_t vl_mat4_sub(vl_mat4_t *dst, const vl_mat4_t a, const  vl_mat4_t b);
VL_API vl_result_t vl_mat4_scale(vl_mat4_t *dst, const vl_mat4_t src, const float k);
VL_API vl_result_t vl_mat4_scale_aniso(vl_mat4_t *dst, const vl_mat4_t src, const vl_vec3_t scale);
VL_API vl_result_t vl_mat4_translate(vl_mat4_t *dst, const vl_mat4_t src, float x, float y, float z);
VL_API vl_result_t vl_mat4_rotate(vl_mat4_t *dst, const vl_mat4_t src, vl_vec3_t axis, float angle);
VL_API vl_result_t vl_mat4_ortho(vl_mat4_t *dst, const float left, const float right, const float bottom, const float top, const float near, const float far);
VL_API vl_vec3_t vl_vec3_norm(vl_vec3_t vec3);

#endif // VELVET_SUPPORT_MATH_H