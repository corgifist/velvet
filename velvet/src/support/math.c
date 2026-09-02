#include "velvet/support/math.h"
#include "support/result.h"
#include "support/memory.h"

vl_result_t vl_mat4_dup(vl_mat4_t *dst, const vl_mat4_t src) {
    if (!dst) return VL_ERROR;
    memcpy(dst, &src, sizeof(src));
    return VL_SUCCESS;
}

vl_result_t vl_mat4_mul(vl_mat4_t *dst, const vl_mat4_t a, const vl_mat4_t b) {
    if (!dst) return VL_ERROR;
	vl_mat4_t temp = {0};
	int k, r, c;
	for(c=0; c<4; ++c) for(r=0; r<4; ++r) {
		temp.m[c].m[r] = 0.f;
		for(k=0; k<4; ++k)
			temp.m[c].m[r] += a.m[k].m[r] * b.m[c].m[k];
	}
	vl_mat4_dup(dst, temp);
    return VL_SUCCESS;
}

vl_result_t vl_mat4_mul_vec4(vl_vec4_t *dst, const vl_mat4_t a, const vl_vec4_t b) {
    if (!dst) return VL_ERROR;
    int i, j;
	for(j=0; j<4; ++j) {
		dst->m[j] = 0.f;
		for(i=0; i<4; ++i)
		    dst->m[j] += a.m[i].m[j] * b.m[i];
	}
    return VL_SUCCESS;
}

vl_result_t vl_mat4_scale(vl_mat4_t *dst, const vl_mat4_t src, const float k) {
    if (!dst) return VL_ERROR;
    for (int i = 0; i < 4; i++) {
        dst->m[i] = VL_VEC4_SCALE(src.m[i], k);
    }
    return VL_SUCCESS;
}

vl_result_t vl_mat4_scale_aniso(vl_mat4_t *dst, const vl_mat4_t src, const vl_vec3_t scale) {
    if (!dst) return VL_ERROR;
    dst->m[0] = VL_VEC4_SCALE(src.m[0], scale.x);
    dst->m[1] = VL_VEC4_SCALE(src.m[1], scale.y);
    dst->m[2] = VL_VEC4_SCALE(src.m[2], scale.z);
    dst->m[3] = src.m[3];
    return VL_SUCCESS;
}

vl_result_t vl_mat4_add(vl_mat4_t *dst, const vl_mat4_t a, const vl_mat4_t b) {
    if (!dst) return VL_ERROR;
    for (int i = 0; i < 4; i++) {
        dst->m[i] = VL_VEC4_ADD(a.m[i], b.m[i]);
    }
    return VL_SUCCESS;
}

vl_result_t vl_mat4_sub(vl_mat4_t *dst, const vl_mat4_t a, const vl_mat4_t b) {
    if (!dst) return VL_ERROR;
    for (int i = 0; i < 4; i++) {
        dst->m[i] = VL_VEC4_SUB(a.m[i], b.m[i]);
    }
    return VL_SUCCESS;
}

vl_result_t vl_mat4_translate(vl_mat4_t *dst, const vl_mat4_t src, float x, float y, float z) {
    if (!dst) return VL_ERROR;
    dst->m[3] = VL_VEC4_ADD(src.m[3], VL_VEC4(x, y, z, 0));
    return VL_SUCCESS;
}

static void mat4_from_vec3_mul_outer(vl_mat4_t *M, vl_vec3_t const a, vl_vec3_t const b)
{
	int i, j;
	for(i=0; i<4; ++i) for(j=0; j<4; ++j)
		M->m[i].m[j] = (i<3 && j<3) ? a.m[i] * b.m[j] : 0.f;
}

vl_result_t vl_mat4_rotate(vl_mat4_t *dst, const vl_mat4_t src, vl_vec3_t axis, float angle) {
	float s = sinf(angle);
	float c = cosf(angle);

	if(VL_VEC3_LEN(axis) > 1e-4) {
        axis = vl_vec3_norm(axis);
        vl_mat4_t T = {0};
		mat4_from_vec3_mul_outer(&T, axis, axis);

		vl_mat4_t S = {{
			{ 0,  axis.m[2], -axis.m[1], 0},
			{-axis.m[2], 0,  axis.m[0], 0},
			{ axis.m[1], -axis.m[0],     0, 0},
			{    0,     0,     0, 0}
        }};
        vl_mat4_scale(&S, S, s);

		vl_mat4_t C = VL_MAT4();
        vl_mat4_sub(&C, C, T);

        vl_mat4_scale(&C, C, c);

		vl_mat4_add(&T, T, C);
		vl_mat4_add(&T, T, S);

		T.m[3].m[3] = 1.f;
		vl_mat4_mul(dst, src, T);
	} else {
		vl_mat4_dup(dst, src);
	}
    return VL_SUCCESS;
}

vl_result_t vl_mat4_ortho(vl_mat4_t *dst, const float l, const float r, const float b, const float t, const float n, const float f) {
	if (!dst) return VL_ERROR;
    dst->m[0].m[0] = 2.f/(r-l);
	dst->m[0].m[1] = dst->m[0].m[2] = dst->m[0].m[3] = 0.f;

	dst->m[1].m[1] = 2.f/(t-b);
	dst->m[1].m[0] = dst->m[1].m[2] = dst->m[1].m[3] = 0.f;

	dst->m[2].m[2] = -2.f/(f-n);
	dst->m[2].m[0] = dst->m[2].m[1] = dst->m[2].m[3] = 0.f;
	
	dst->m[3].m[0] = -(r+l)/(r-l);
	dst->m[3].m[1] = -(t+b)/(t-b);
	dst->m[3].m[2] = -(f+n)/(f-n);
	dst->m[3].m[3] = 1.f;
    return VL_SUCCESS;
}

vl_vec3_t vl_vec3_norm(vl_vec3_t vec3) {
    float length = VL_VEC3_LEN(vec3);
    if (length == 0.0f) {
        return VL_VEC3(0);
    }
    return VL_VEC3(
        vec3.x / length,
        vec3.y / length,
        vec3.z / length
    );
}