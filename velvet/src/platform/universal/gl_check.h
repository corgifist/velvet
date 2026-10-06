#ifndef VELVET_PLATFORM_UNIVERSAL_GL_CHECK_H
#define VELVET_PLATFORM_UNIVERSAL_GL_CHECK_H

#include "support/memory.h"
#include <glad/gl.h>

#define VL_GL_CALL(CTX, CALL) \
    do { \
        (CTX) . CALL; \
        vl_gl_check_errors(&(CTX), VL_HERE); \
    } while (0)
#define GL_CALL VL_GL_CALL

void vl_gl_check_errors(GladGLContext *ctx, vl_source_location_t loc);
void vl_gl_drain_errors(GladGLContext *ctx);

#endif // VELVET_PLATFORM_UNIVERSAL_GL_CHECK_H