#include "gl_check.h"
#include "support/managed_assert.h"
#include <stdio.h>
#include <stdlib.h>

void vl_gl_check_errors(GladGLContext *ctx) {
    GLenum error = ctx->GetError();
    if (error == GL_NO_ERROR) return;
    printf("GL ERROR: %i\n", (int) error);
    VL_ASSERT(0);
}

void vl_gl_drain_errors(GladGLContext *ctx) {
    while (ctx->GetError() != GL_NO_ERROR) {}
}