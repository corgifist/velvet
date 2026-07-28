#ifndef VELVET_PLATFORM_UNIVERSAL_GL_CHECK_H
#define VELVET_PLATFORM_UNIVERSAL_GL_CHECK_H

#include <glad/gl.h>

#define VL_GL_CALL(CTX, CALL) \
    do { \
        (CTX) . CALL; \
        vl_gl_check_errors(&(CTX)); \
    } while (0)
#define GL_CALL VL_GL_CALL

void vl_gl_check_errors(GladGLContext *ctx);
void vl_gl_drain_errors(GladGLContext *ctx);

#endif // VELVET_PLATFORM_UNIVERSAL_GL_CHECK_H