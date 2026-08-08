#include "font/atlas.h"
#include "support/result.h"
#include "support/memory.h"
#include "font/font.h"

vl_font_atlas_t *vl_font_atlas_new_(vl_font_atlas_format_t format, size_t width, size_t height, vl_source_location_t loc) {
    vl_font_atlas_t *atlas = VL_NEW(vl_font_atlas_t, loc);
    if (!atlas) goto err;
    if (vl_font_atlas_init(atlas, format, width, height, loc)) goto err;

    return atlas;
    err:
    vl_free(atlas);
    return NULL;
}

static size_t get_pixel_size(vl_font_atlas_format_t format) {
    switch (format) {
    case VL_FONT_ATLAS_FORMAT_RRRR8: return sizeof(uint8_t);
    default: return sizeof(uint8_t);
    }
}

vl_result_t vl_font_atlas_init_(vl_font_atlas_t *atlas, vl_font_atlas_format_t format, size_t width, size_t height, vl_source_location_t loc) {
    if (!atlas) return VL_ERROR;
    atlas->format = format;
    atlas->width = width;
    atlas->height = height;
    atlas->data = vl_malloc(width * height * get_pixel_size(format), loc);
    atlas->cursor_x = 0;
    atlas->cursor_y = 0;
    atlas->largest_glyph_on_line = 0;
    atlas->codepoints = VL_DA_INIT(vl_font_atlas_codepoint_t);
    return VL_SUCCESS;
}

vl_font_atlas_codepoint_t *vl_font_atlas_find_glyph_id(vl_font_atlas_t *atlas, vl_font_t *font, uint32_t glyph_id) {
    for (int i = 0; i < VL_DA_LENGTH(atlas->codepoints); i++) {
        vl_font_atlas_codepoint_t *c = atlas->codepoints + i;
        if (!c->owner) continue; 
        if (c->owner == font && c->glyph_id == glyph_id) {
            return c;
        }
    }
    return NULL;
}

vl_result_t vl_font_atlas_deinit(vl_font_atlas_t *atlas) {
    if (!atlas) return VL_ERROR;
    vl_free(atlas->data);
    memset(atlas, 0, sizeof(*atlas));
    return VL_SUCCESS;
}

vl_result_t vl_font_atlas_free(vl_font_atlas_t *atlas) {
    if (!atlas) return VL_ERROR;
    vl_font_atlas_deinit(atlas);
    VL_DA_FREE(atlas->codepoints);
    vl_free(atlas);
    return VL_SUCCESS;
}