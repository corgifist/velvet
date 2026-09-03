#include "velvet/web/fonts.h"
#include "font/atlas.h"
#include "font/font.h"
#include "font/search.h"
#include "font/shaper.h"
#include "graphics/bitmap.h"
#include "graphics/brush.h"
#include "support/da.h"
#include "support/global_error_pool.h"
#include "support/io.h"
#include "support/memory.h"
#include "support/result.h"
#include "vendor/utf8.h"
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

VL_API vl_result_t vl_web_fonts_add_font(vl_web_fonts_t *fonts, const char *family_name, const vl_byte_t *font_data, size_t font_len, vl_web_font_weight_t weight) {
    return vl_web_fonts_add_font_with_part_name(fonts, family_name, font_data, font_len, weight, NULL);
}

vl_result_t vl_web_fonts_add_font_with_part_name(vl_web_fonts_t *fonts, const char *family_name, const vl_byte_t *font_data, size_t font_len, vl_web_font_weight_t weight, const char *part_name) {
    if (!fonts || !family_name || !font_data) return VL_ERROR;
    vl_web_font_family_t *family = find_family(fonts, family_name);
    if (!family) {
        family = VL_DA_PUSH(fonts->families, vl_web_font_family_t);
        family->name = family_name;
        family->variations = VL_DA_INIT(vl_web_font_t);
    }

    vl_web_font_t *variation = find_variation(family, weight);
    if (!variation) {
        variation = VL_DA_PUSH(family->variations, vl_web_font_t);
        variation->weight = weight;
        variation->parts = VL_DA_INIT(vl_web_font_part_t);
    }
    vl_web_font_part_t part = {0};
    part.data = font_data;
    part.len = font_len;
    part.name = part_name ? part_name : family_name;
    part.unit_font = vl_font_new(fonts->owner->platform_context, part.name, 1, 1.0f, font_data, font_len);
    part.unit_shaper_ref = vl_font_shaper_add_font(fonts->shaper, part.unit_font);
    part.sized_fonts = VL_DA_INIT(vl_web_sized_font_t);
    VL_DA_APPEND(variation->parts, part);
    return VL_SUCCESS;
}

vl_result_t vl_web_fonts_add_font_with_part_name_from_disk(vl_web_fonts_t *fonts, const char *family_name, const char *path, vl_web_font_weight_t weight, const char *part_name) {
    if (!fonts || !family_name || !path) return VL_ERROR;
    vl_web_font_family_t *family = find_family(fonts, family_name);
    if (!family) {
        family = VL_DA_PUSH(fonts->families, vl_web_font_family_t);
        family->name = family_name;
        family->variations = VL_DA_INIT(vl_web_font_t);
    }

    vl_web_font_t *variation = find_variation(family, weight);
    if (!variation) {
        variation = VL_DA_PUSH(family->variations, vl_web_font_t);
        variation->weight = weight;
        variation->parts = VL_DA_INIT(vl_web_font_part_t);
    }

    vl_web_font_part_t part = {0};
    part.path = path;
    part.name = part_name ? part_name : family_name;
    part.sized_fonts = VL_DA_INIT(vl_web_sized_font_t);
    VL_DA_APPEND(variation->parts, part);
    return VL_SUCCESS;
}

static vl_web_font_weight_t classify_weight(const char *font_name) {
    vl_web_font_weight_t result = VL_WEB_FONT_REGULAR;
    if (utf8casestr(font_name, "ExtraLight") || utf8casestr(font_name, "Extra Light")) 
        result = VL_WEB_FONT_EXTRA_LIGHT;
    if (utf8casestr(font_name, "Light")) result = VL_WEB_FONT_LIGHT;
    if (utf8casestr(font_name, "Bold")) result = VL_WEB_FONT_BOLD;
    return result;
}

VL_API vl_result_t vl_web_fonts_add_family_from_system(vl_web_fonts_t *fonts, const char *family_name) {
    if (!fonts || !family_name) return VL_ERROR;
    VL_DA(vl_font_search_description_t) search = VL_DA_INIT(vl_font_search_description_t);
    vl_font_search_query(&search, family_name);
    for (int i = 0; i < VL_DA_LENGTH(search); i++) {
        vl_font_search_description_t *font = search + i;
        if (utf8str(font->name, "Italic")) continue;
        vl_web_font_weight_t weight = classify_weight(font->name);
        // printf("found font: %s %s %i\n", font->name, font->path, weight);
        vl_web_fonts_add_font_with_part_name_from_disk(fonts, family_name, VL_DA_COPY(font->path), weight, VL_DA_COPY(font->name));
    }
    VL_DA_FREE(search);
    return VL_SUCCESS;
}

static void prepare_part(vl_web_fonts_t *fonts, vl_web_font_part_t *part) {
    if (!part) return;
    if (part->path && !part->data) {
        part->data = vl_io_read_file(part->path);
        // printf("read data: %s %p\n", part->path, part->data);
        if (!part->data) return;
        part->len = VL_DA_LENGTH(part->data) - 1;
        part->unit_font = vl_font_new(fonts->owner->platform_context, part->name, 1, 1, part->data, part->len);
        part->unit_shaper_ref = vl_font_shaper_add_font(fonts->shaper, part->unit_font);
    }
}

VL_DA(vl_web_sized_font_t*) vl_web_fonts_get_font(vl_web_fonts_t *fonts, const char *family_name, vl_web_font_weight_t weight, int height) {
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

    VL_DA(vl_web_sized_font_t*) result = NULL;
    for (int i = 0; i < VL_DA_LENGTH(variation->parts); i++) {
        if (!result) result = VL_DA_INIT(vl_web_sized_font_t*);
        vl_web_font_part_t *part = variation->parts + i;
        prepare_part(fonts, part);
        vl_web_sized_font_t *sized_font = NULL;
        for (int j = 0; j < VL_DA_LENGTH(part->sized_fonts); j++) {
            if (part->sized_fonts[j].font->height == height) {
                sized_font = part->sized_fonts + j;
                break;
            }
        }
        
        if (!sized_font) {
            sized_font = VL_DA_PUSH(part->sized_fonts, vl_web_sized_font_t);
            sized_font->font = vl_font_new(fonts->owner->platform_context, part->name, height, 2.0f, part->data, part->len);
            sized_font->shaper_ref = part->unit_shaper_ref;
        }

        VL_DA_APPEND(result, sized_font);
    }
    return result;
}

vl_web_sized_font_t *vl_web_fonts_get_font_by_unit_font(vl_web_fonts_t *fonts, vl_font_t *unit_font, vl_web_font_weight_t weight, int height) {
    if (!fonts || !unit_font) return NULL;
    for (int i = 0; i < VL_DA_LENGTH(fonts->families); i++) {
        vl_web_font_family_t *family = fonts->families + i;
        for (int j = 0; j < VL_DA_LENGTH(family->variations); j++) {
            vl_web_font_t *variation = family->variations + j;
            if (variation->weight != weight) continue;
            for (int k = 0; k < VL_DA_LENGTH(variation->parts); k++) {
                vl_web_font_part_t *part = variation->parts + k;
                prepare_part(fonts, part);
                if (part->unit_font == unit_font) {
                    for (int l = 0; l < VL_DA_LENGTH(part->sized_fonts); l++) {
                        vl_web_sized_font_t *sized_font = part->sized_fonts + l;
                        if (sized_font->font->height == height) {
                            return sized_font;
                        }
                    }
                    vl_web_sized_font_t *sized_font = VL_DA_PUSH(part->sized_fonts, vl_web_sized_font_t);
                    sized_font->font = vl_font_new(fonts->owner->platform_context, part->name, height, 2.0f, part->data, part->len);
                    sized_font->shaper_ref = part->unit_shaper_ref;
                    VL_DA_APPEND(part->sized_fonts, sized_font);
                    return sized_font;
                }
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
            for (int k = 0; k < VL_DA_LENGTH(variation->parts); k++) {
                vl_web_font_part_t *part = variation->parts + k;
                for (int l = 0; l < VL_DA_LENGTH(part->sized_fonts); l++) {
                    vl_web_sized_font_t *sized_font = part->sized_fonts + l;
                    vl_font_free(sized_font->font);
                }
                VL_DA_FREE(part->sized_fonts);
                vl_font_shaper_free_font(fonts->shaper, part->unit_shaper_ref);
                vl_font_free(part->unit_font);
            }
            VL_DA_FREE(variation->parts);
        }
        VL_DA_FREE(family->variations);
    }
    VL_DA_FREE(fonts->families);

    vl_font_shaper_free(fonts->shaper);
    return VL_SUCCESS;
}