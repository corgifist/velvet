#include "gl_check.h"
#include <stdio.h>

void vl_gl_check_errors(GladGLContext *ctx) {
    GLenum error = ctx->GetError();
    if (error == GL_NO_ERROR) return;
    printf("GL ERROR: %i\n", (int) error);
}

void vl_gl_drain_errors(GladGLContext *ctx) {
    while (ctx->GetError() != GL_NO_ERROR) {}
}