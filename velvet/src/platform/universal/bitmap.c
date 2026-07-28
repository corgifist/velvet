#include "velvet/platform/universal/bitmap.h"
#include "graphics/bitmap.h"
#include "platform/context.h"
#include "velvet/platform/universal/render.h"
#include "gl_check.h"

GLint get_internal_format(vl_graphics_bitmap_format_t format) {
    switch (format) {
    case VL_GRAPHICS_BITMAP_FORMAT_RGBA8:
        return GL_RGBA8;
    default: {
        return GL_RGB;
    }
    }
}

GLint get_format(vl_graphics_bitmap_format_t format) {
    switch (format) {
    case VL_GRAPHICS_BITMAP_FORMAT_RGBA8:
        return GL_RGBA;
    default: {
        return GL_RGBA;
    }
    }
}

GLint get_type(vl_graphics_bitmap_format_t format) {
    switch (format) {
    case VL_GRAPHICS_BITMAP_FORMAT_RGBA8:
        return GL_UNSIGNED_BYTE;
    default: {
        return GL_UNSIGNED_BYTE;
    }
    }
}

vl_graphics_bitmap_t *vl_graphics_bitmap_universal_new(vl_graphics_render_t *render, 
        size_t width, size_t height, vl_graphics_bitmap_format_t format, void *data) {
    if (!render) return NULL;
    vl_graphics_bitmap_universal_t *bitmap = VL_NEW(vl_graphics_bitmap_universal_t);
    if (!bitmap) return NULL;
    vl_graphics_render_universal_t *r = ((vl_graphics_render_universal_t*) render);
    GL_CALL(r->ctx, GenTextures(1, &bitmap->handle));
    GL_CALL(r->ctx, BindTexture(GL_TEXTURE_2D, bitmap->handle));
    GL_CALL(r->ctx, TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(r->ctx, TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(r->ctx, TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CALL(r->ctx, TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CALL(r->ctx, TexImage2D(
        GL_TEXTURE_2D, 0, get_internal_format(format), 
            width, height, 0, get_format(format), get_type(format), data));
    // r->ctx.GenerateMipmap(GL_TEXTURE_2D);
    return (vl_graphics_bitmap_t*) bitmap;
}

vl_result_t vl_graphics_bitmap_universal_free(vl_graphics_bitmap_t *bitmap) {
    if (!bitmap || !bitmap->owner || !bitmap->owner->context 
        || bitmap->owner->context->types.graphics_render != VL_PLATFORM_CONTEXT_UNIVERSAL) return VL_ERROR;
    vl_graphics_bitmap_universal_t *b = (vl_graphics_bitmap_universal_t*) bitmap;
    vl_graphics_render_universal_t *r = (vl_graphics_render_universal_t*) bitmap->owner;
    GL_CALL(r->ctx, DeleteTextures(1, &b->handle));
    b->handle = 0;
    vl_free(b);
    return VL_SUCCESS;
}