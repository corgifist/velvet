#include "velvet/web/fonts.h"
#include "font/atlas.h"
#include "font/font.h"
#include "font/shaper.h"
#include "graphics/bitmap.h"
#include "graphics/brush.h"
#include "support/da.h"
#include "support/global_error_pool.h"
#include "support/memory.h"
#include "support/result.h"
#include "graphics/render.h"
#include "web/web.h"
#include <stdint.h>

vl_result_t vl_web_fonts_init(vl_web_fonts_t *fonts, vl_web_t *web) {
    if (!fonts) return VL_ERROR;
    fonts->owner = NULL;
    fonts->families = VL_DA_INIT(vl_web_font_family_t);
    fonts->atlases = VL_DA_INIT(vl_web_font_atlas_t);
    fonts->shaper = vl_font_shaper_new(web->platform_context);
    return VL_SUCCESS;
}

static vl_web_font_family_t *find_family(vl_web_fonts_t *fonts, const char *family_name) {
    for (int i = 0; i < VL_DA_LENGTH(fonts->families); i++) {
        vl_web_font_family_t *family = fonts->families + i;
        if (strcmp(family_name, family->name) == 0) {
            return family;
        }
    }
    return NULL;
}

static vl_web_font_t *find_variation(vl_web_font_family_t *family, vl_web_font_weight_t weight) {
    for (int i = 0; i < VL_DA_LENGTH(family->variations); i++) {
        vl_web_font_t *variation = family->variations + i;
        if (variation->weight == weight) {
            return variation;
        }
    }
    return NULL;
}

vl_result_t vl_web_fonts_add_font(vl_web_fonts_t *fonts, const char *family_name, const vl_byte_t *font_data, size_t font_len, vl_web_font_weight_t weight) {
    if (!fonts || !family_name || !font_data) return VL_ERROR;
    vl_web_font_family_t *family = find_family(fonts, family_name);
    if (!family) {
        family = VL_DA_PUSH(fonts->families, vl_web_font_family_t);
        family->name = family_name;
        family->variations = VL_DA_INIT(vl_web_font_t);
    }

    vl_web_font_t *variation = find_variation(family, weight);
    if (variation) {
        for (int i = 0; i < VL_DA_LENGTH(variation->sizes); i++) {
            vl_font_free(variation->sizes[i].font);
        }
        VL_DA_FREE(variation->sizes);
    }
    variation = VL_DA_PUSH(family->variations, vl_web_font_t);
    variation->font_data = font_data;
    variation->font_len = font_len;
    variation->weight = weight;
    variation->sizes = VL_DA_INIT(vl_web_sized_font_t);
    variation->unit_font = vl_font_new(fonts->owner->platform_context, family_name, 1, 1.0f, font_data, font_len);
    variation->unit_font_ref = vl_font_shaper_add_font(fonts->shaper, variation->unit_font);
    return VL_SUCCESS;
}

vl_web_sized_font_t *vl_web_fonts_get_font(vl_web_fonts_t *fonts, const char *family_name, vl_web_font_weight_t weight, int height) {
    if (!fonts || !family_name) return NULL;
    vl_web_font_family_t *family = find_family(fonts, family_name);
    if (!family) {
        vl_global_error_pool_append("no such font family '%s' for vl_web_fonts_t %p", family_name, fonts);
        return NULL;
    }

    vl_web_font_t *variation = find_variation(family, weight);
    if (!variation) {
        vl_global_error_pool_append("no such font variation with weight %i for family with name %s for vl_web_fonts_t %p", weight, family->name, fonts);
        return NULL;
    }

    vl_web_sized_font_t *sized_font = NULL;
    for (int i = 0; i < VL_DA_LENGTH(variation->sizes); i++) {
        if (variation->sizes[i].font->height == height) {
            sized_font = variation->sizes + i;
            break;
        }
    }

    if (!sized_font) {
        vl_font_t *font = vl_font_new(fonts->owner->render->context, family_name, height, 2.0f, variation->font_data, variation->font_len);
        vl_web_sized_font_t sf = {0};
        sf.font = font;
        sf.shaper_ref = variation->unit_font_ref;
        sized_font = VL_DA_APPEND(variation->sizes, sf);
    }
    return sized_font;
}

vl_web_sized_font_t *vl_web_fonts_get_font_by_unit_font(vl_web_fonts_t *fonts, vl_font_t *unit_font, vl_web_font_weight_t weight, int height) {
    if (!fonts || !unit_font) return NULL;
    for (int i = 0; i < VL_DA_LENGTH(fonts->families); i++) {
        vl_web_font_family_t *family = fonts->families + i;
        for (int j = 0; j < VL_DA_LENGTH(family->variations); j++) {
            vl_web_font_t *variation = family->variations + j;
            if (unit_font == variation->unit_font) {
                return vl_web_fonts_get_font(fonts, family->name, weight, height);
            }
        }
    }
    return NULL;
}

static vl_byte_t *s_tmp_copy_buffer = NULL;
static size_t s_tmp_copy_size = 0;

static vl_result_t rasterize_glyph_id(vl_web_fonts_t *fonts, vl_web_font_atlas_codepoint_t *codepoint, vl_font_t *font, uint32_t glyph_id) {
    // first check if the glyph is already rasterized
    for (int i = 0; i < VL_DA_LENGTH(fonts->atlases); i++) {
        vl_web_font_atlas_t *atlas = fonts->atlases + i;
        vl_font_atlas_codepoint_t *search = vl_font_atlas_find_glyph_id(&atlas->atlas, font, glyph_id);
        if (search) {
            if (codepoint) {
                codepoint->atlas = atlas;
                codepoint->codepoint = search;
            }
            return VL_SUCCESS;
        }
    }

    vl_web_font_atlas_t *free_atlas = NULL;
    for (int i = 0; i < VL_DA_LENGTH(fonts->atlases); i++) {
        if (!fonts->atlases[i].atlas.full) {
            free_atlas = fonts->atlases + i;
            break;
        }
    }
    if (!free_atlas) {
        free_atlas = VL_DA_PUSH(fonts->atlases, vl_web_font_atlas_t);
        vl_font_atlas_init(&free_atlas->atlas, VL_FONT_ATLAS_FORMAT_RRRR8, 512, 512);
        free_atlas->bitmap = vl_graphics_bitmap_new(fonts->owner->render, free_atlas->atlas.width, free_atlas->atlas.height, VL_GRAPHICS_BITMAP_FORMAT_RRRR8, NULL);
        free_atlas->brush = vl_graphics_brush_new_bitmap(fonts->owner->render, free_atlas->bitmap);
    }
    
    vl_font_atlas_codepoint_t *rasterized = vl_font_rasterize_glyph_id(font, &free_atlas->atlas, glyph_id);
    if (!rasterized) return VL_ERROR;
    size_t cursor_x = rasterized->uv.tl.x * free_atlas->atlas.width;
    size_t cursor_y = rasterized->uv.tl.y * free_atlas->atlas.height;
    size_t w = rasterized->w * font->density;
    size_t h = rasterized->h * font->density;
    if (!s_tmp_copy_buffer || s_tmp_copy_size < w * h) {
        s_tmp_copy_buffer = realloc(s_tmp_copy_buffer, w * h);
        s_tmp_copy_size = w * h;
    }
    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
            size_t index = ((y + cursor_y) * free_atlas->atlas.width) + cursor_x + x;
            s_tmp_copy_buffer[y * w + x] = free_atlas->atlas.data[index];
        }
    }
    vl_graphics_bitmap_update(free_atlas->bitmap, cursor_x, cursor_y, w, h, s_tmp_copy_buffer);
    if (codepoint) {
        codepoint->codepoint = rasterized;
        codepoint->atlas = free_atlas;
    }
    return VL_SUCCESS;
}

vl_result_t vl_web_fonts_find_glyph_id_with_font(vl_web_fonts_t *fonts, vl_web_font_atlas_codepoint_t *codepoint, vl_font_t *font, uint32_t glyph_id) {
    if (!fonts || !font) return VL_ERROR;
    return rasterize_glyph_id(fonts, codepoint, font, glyph_id);
}

vl_result_t vl_web_fonts_find_glyph_id(vl_web_fonts_t *fonts, vl_web_font_atlas_codepoint_t *codepoint, const char *family_name, vl_web_font_weight_t weight, int height, uint32_t glyph_id) {
    if (!fonts || !family_name) return VL_ERROR;

    vl_web_sized_font_t *sized_font = vl_web_fonts_get_font(fonts, family_name, weight, height);
    if (!sized_font) return VL_ERROR;

    return vl_web_fonts_find_glyph_id_with_font(fonts, codepoint, sized_font->font, glyph_id);
}

vl_result_t vl_web_fonts_deinit(vl_web_fonts_t *fonts) {
    if (!fonts) return VL_ERROR;
    for (int i = 0; i < VL_DA_LENGTH(fonts->atlases); i++) {
        vl_web_font_atlas_t *atlas = fonts->atlases + i;
        vl_font_atlas_deinit(&atlas->atlas);
        vl_graphics_brush_free(atlas->brush);
        vl_graphics_bitmap_free(atlas->bitmap);
    }
    VL_DA_FREE(fonts->atlases);
    for (int i = 0; i < VL_DA_LENGTH(fonts->families); i++) {
        vl_web_font_family_t *family = fonts->families + i;
        for (int j = 0; j < VL_DA_LENGTH(family->variations); j++) {
            vl_web_font_t *variation = family->variations + j;
            for (int k = 0; k < VL_DA_LENGTH(variation->sizes); k++) {
                vl_web_sized_font_t *size = variation->sizes + k;
                vl_font_free(size->font);
            }
            vl_font_shaper_free_font(fonts->shaper, variation->unit_font_ref);
            vl_font_free(variation->unit_font);
            VL_DA_FREE(variation->sizes);
        }
        VL_DA_FREE(family->variations);
    }
    VL_DA_FREE(fonts->families);

    vl_font_shaper_free(fonts->shaper);
    return VL_SUCCESS;
}