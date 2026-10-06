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
#include "web/font_storage.h"
#include "web/web.h"
#include "support/base_math.h"
#include <limits.h>

vl_result_t vl_web_fonts_init(vl_web_fonts_t *fonts, vl_web_t *web) {
    if (!fonts) return VL_ERROR;
    VL_ZERO_OUT(fonts);
    fonts->owner = NULL;
    fonts->families = VL_DA_INIT(vl_web_font_family_t);
    fonts->atlases = VL_DA_INIT(vl_web_font_atlas_t);
    fonts->shaper = vl_font_shaper_new(web->platform_context);
    vl_web_font_storage_init(&fonts->storage);
    vl_font_search_query(&fonts->system_fonts, NULL);
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

static VL_DA(vl_web_font_t*) find_variation(vl_web_font_family_t *family, vl_web_font_weight_t weight) {
    VL_DA(vl_web_font_t*) variations = NULL;
    for (int i = 0; i < VL_DA_LENGTH(family->variations); i++) {
        vl_web_font_t *variation = family->variations + i;
        if (weight > 0 && variation->weight != weight) continue;
        if (!variations) variations = VL_DA_INIT(vl_web_font_t*);
        VL_DA_APPEND(variations, variation);
    }
    return variations;
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

    VL_DA(vl_web_font_t*) variations = find_variation(family, weight);
    vl_web_font_t *variation = variations ? *variations : NULL;
    if (!variations) {
        variation = VL_DA_PUSH(family->variations, vl_web_font_t);
        variation->weight = weight;
        variation->parts = VL_DA_INIT(vl_web_font_part_t);
    }
    vl_web_font_part_t part = {0};
    part.data = font_data;
    part.len = font_len;
    part.name = part_name ? part_name : family_name;
    part.sized_fonts = VL_DA_INIT(vl_web_sized_font_t);
    VL_DA_APPEND(variation->parts, part);
    VL_DA_FREE(variations);
    return VL_SUCCESS;
}

vl_result_t vl_web_fonts_add_font_with_part_name_from_disk(vl_web_fonts_t *fonts, const char *family_name, const char *path, vl_web_font_weight_t weight, const char *part_name, VL_DA(int) subfonts, int priority) {
    if (!fonts || !family_name || !path) return VL_ERROR;
    vl_web_font_family_t *family = find_family(fonts, family_name);
    if (!family) {
        family = VL_DA_PUSH(fonts->families, vl_web_font_family_t);
        family->name = family_name;
        family->variations = VL_DA_INIT(vl_web_font_t);
        family->priority = priority;
    }

    VL_DA(vl_web_font_t*) variations = find_variation(family, weight);
    vl_web_font_t *variation = variations ? *variations : NULL;
    if (!variation) {
        variation = VL_DA_PUSH(family->variations, vl_web_font_t);
        variation->weight = weight;
        variation->parts = VL_DA_INIT(vl_web_font_part_t);
    }

    vl_web_font_part_t part = {0};
    part.path = path;
    part.name = part_name ? part_name : family_name;
    part.sized_fonts = VL_DA_INIT(vl_web_sized_font_t);
    part.subfonts = subfonts;
    part.priority = priority;
    // printf("adding %s %s (%s) %i\n", family_name, part_name, path, weight);
    VL_DA_APPEND(variation->parts, part);
    VL_DA_FREE(variations);
    return VL_SUCCESS;
}

static void add_system_font_from_description(vl_web_fonts_t *fonts, vl_font_search_description_t *font, const char *family_name, const char *part_family, int priority) {
    // printf("%s %s %s %i\n", font->name, family_name, part_family, vl_font_search_compare_family_names(font->name, part_family));
    if (!vl_font_search_compare_family_names(font->name, part_family)) return;
    if (utf8casestr(font->name, "Italic") || utf8casestr(font->name, "Narrow") || utf8casestr(font->name, "Condensed")) return;
    int weight;
    vl_font_search_description_print(font);
    vl_font_search_classify(font->name, &weight, NULL, NULL, NULL, NULL);
    vl_web_fonts_add_font_with_part_name_from_disk(fonts, family_name, font->path, weight, font->name, font->subfonts, priority);
}

vl_result_t vl_web_fonts_add_parts_from_system(vl_web_fonts_t *fonts, const char *family_name, const char *part_family) {
    if (!fonts || !family_name || !part_family) return VL_ERROR;
    vl_web_font_family_t *family = find_family(fonts, family_name);
    if (!family) {
        family = VL_DA_PUSH(fonts->families, vl_web_font_family_t);
        family->name = family_name;
        family->variations = VL_DA_INIT(vl_web_font_t);
    }
    int priority = ++fonts->priority_index;
    for (int i = 0; i < VL_DA_LENGTH(fonts->system_fonts); i++) {
        vl_font_search_description_t *font = fonts->system_fonts + i;
        add_system_font_from_description(fonts, font, family_name, part_family, priority);
    }

    return VL_SUCCESS;
}

VL_API vl_result_t vl_web_fonts_add_parts_from_disk(vl_web_fonts_t *fonts, const char *family_name, const char *path) {
    if (!fonts || !family_name || !path) return VL_ERROR;
    vl_web_font_family_t *family = find_family(fonts, family_name);
    if (!family) {
        family = VL_DA_PUSH(fonts->families, vl_web_font_family_t);
        family->name = family_name;
        family->variations = VL_DA_INIT(vl_web_font_t);
    }
    int priority = ++fonts->priority_index;
    vl_web_font_storage_record_t *query = vl_web_font_storage_query(&fonts->storage, path);
    if (!query) return VL_ERROR;
    vl_font_t *fat_font = vl_font_new(fonts->owner->platform_context, family_name, 1, 1, query->data, query->len);
    if (!fat_font) return VL_ERROR;
    for (int i = 0; i < VL_DA_LENGTH(fat_font->fonts); i++) {
        vl_font_info_t *subfont = fat_font->fonts[i];
        if (!subfont) continue;
        if (utf8casestr(subfont->name, "Italic")) continue;
        int weight;
        vl_font_search_classify(subfont->name, &weight, NULL, NULL, NULL, NULL);
        VL_DA(int) subfonts = VL_DA_INIT(int);
        *VL_DA_PUSH(subfonts, int) = i;
        vl_web_fonts_add_font_with_part_name_from_disk(fonts, family_name, path, weight, VL_STRING_COPY(subfont->name), subfonts, priority);
    }
    vl_font_free(fat_font);

    return VL_SUCCESS;
}

VL_API vl_result_t vl_web_fonts_add_family_from_system(vl_web_fonts_t *fonts, const char *family_name) {
    if (!fonts || !family_name) return VL_ERROR;
    int priority = ++fonts->priority_index;
    for (int i = 0; i < VL_DA_LENGTH(fonts->system_fonts); i++) {
        vl_font_search_description_t *font = fonts->system_fonts + i;
        add_system_font_from_description(fonts, font, family_name, family_name, priority);
    }
    return VL_SUCCESS;
}

static void prepare_part(vl_web_fonts_t *fonts, vl_web_font_part_t *part) {
    if (!part) return;
    if (part->path && !part->data) {
        vl_web_font_storage_record_t *record = vl_web_font_storage_query(&fonts->storage, part->path);
        if (!record) return;
        part->data = record->data;
        part->len = record->len;
        // printf("read data: %s %p\n", part->path, part->data);
        if (!part->data) return;
        part->len = VL_DA_LENGTH(part->data) - 1;
    }
}

VL_DA(vl_web_sized_font_t*) vl_web_fonts_get_font(vl_web_fonts_t *fonts, const char *family_name, vl_web_font_weight_t weight, int height) {
    if (!fonts || !family_name) return NULL;
    vl_web_font_family_t *family = find_family(fonts, family_name);
    if (!family) {
        vl_global_error_pool_append("no such font family '%s' for vl_web_fonts_t %p", family_name, fonts);
        return NULL;
    }

    VL_DA(vl_web_font_t*) variations = find_variation(family, weight);
    if (!variations) {
        vl_global_error_pool_append("no such font variation with weight %i for family with name %s for vl_web_fonts_t %p", weight, family->name, fonts);
        return NULL;
    }

    VL_DA(vl_web_sized_font_t*) result = NULL;
    for (int l = 0; l < VL_DA_LENGTH(variations); l++) {
        vl_web_font_t *variation = variations[l];
        for (int i = 0; i < VL_DA_LENGTH(variation->parts); i++) {
            if (!result) result = VL_DA_INIT(vl_web_sized_font_t*);
            vl_web_font_part_t *part = variation->parts + i;
            prepare_part(fonts, part);
            vl_web_sized_font_t *sized_font = NULL;
            for (int j = 0; j < VL_DA_LENGTH(part->sized_fonts); j++) {
                if (!part->sized_fonts[j].font) continue;
                if (part->sized_fonts[j].font->height == height) {
                    sized_font = part->sized_fonts + j;
                    break;
                }
            }
            
            if (!sized_font) {
                sized_font = VL_DA_PUSH(part->sized_fonts, vl_web_sized_font_t);
                sized_font->font = vl_font_new_with_subfont_indices(fonts->owner->platform_context, part->name, height, 2.0f, part->data, part->len, part->subfonts);
                sized_font->shaper_ref = vl_font_shaper_add_font(fonts->shaper, sized_font->font);
                sized_font->weight = variation->weight;
                sized_font->priority = part->priority;
            }

            VL_DA_APPEND(result, sized_font);
        }
    }
    VL_DA_FREE(variations);
    return result;
}

static vl_byte_t *s_tmp_copy_buffer = NULL;
static size_t s_tmp_copy_size = 0;

static vl_result_t rasterize_glyph_id(vl_web_fonts_t *fonts, vl_web_font_atlas_codepoint_t *codepoint, vl_font_t *font, uint32_t glyph_id, int font_index) {
    // first check if the glyph is already rasterized
    for (int i = 0; i < VL_DA_LENGTH(fonts->atlases); i++) {
        vl_web_font_atlas_t *atlas = fonts->atlases + i;
        vl_font_atlas_codepoint_t *search = vl_font_atlas_find_glyph_id_with_font_index(&atlas->atlas, font, glyph_id, font_index);
        if (search) {
            if (codepoint) {
                codepoint->atlas_index = i;
                codepoint->codepoint_index = search->index;
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
        vl_font_atlas_init(&free_atlas->atlas, VL_FONT_ATLAS_FORMAT_RRRR8, 2048, 1024);
        free_atlas->bitmap = vl_graphics_bitmap_new(fonts->owner->render, free_atlas->atlas.width, free_atlas->atlas.height, VL_GRAPHICS_BITMAP_FORMAT_RRRR8, NULL);
        free_atlas->brush = vl_graphics_brush_new_bitmap(fonts->owner->render, free_atlas->bitmap);
        free_atlas->index = VL_DA_LENGTH(fonts->atlases) - 1;
    }
    
    vl_font_atlas_codepoint_t *rasterized = vl_font_rasterize_glyph_id_with_font_index(font, &free_atlas->atlas, glyph_id, font_index);
    if (!rasterized) {
        if (free_atlas->atlas.full) return rasterize_glyph_id(fonts, codepoint, font, glyph_id, font_index);
        else return VL_ERROR;
    }
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
        codepoint->codepoint_index = rasterized->index;
        codepoint->atlas_index = free_atlas->index;
    }
    return VL_SUCCESS;
}

vl_result_t vl_web_fonts_find_glyph_id_with_font_and_font_index(vl_web_fonts_t *fonts, vl_web_font_atlas_codepoint_t *codepoint, vl_font_t *font, uint32_t glyph_id, int font_index) {
    if (!fonts || !font) return VL_ERROR;
    return rasterize_glyph_id(fonts, codepoint, font, glyph_id, font_index);
}

vl_result_t vl_web_fonts_find_glyph_id_with_font(vl_web_fonts_t *fonts, vl_web_font_atlas_codepoint_t *codepoint, vl_font_t *font, uint32_t glyph_id) {
    if (!fonts || !font) return VL_ERROR;
    return rasterize_glyph_id(fonts, codepoint, font, glyph_id, 0);
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
                    vl_font_shaper_free_font(fonts->shaper, sized_font->shaper_ref);
                    vl_font_free(sized_font->font);
                }
                VL_DA_FREE(part->sized_fonts);
            }
            VL_DA_FREE(variation->parts);
        }
        VL_DA_FREE(family->variations);
    }
    VL_DA_FREE(fonts->families);
    vl_web_font_storage_deinit(&fonts->storage);
    vl_font_shaper_free(fonts->shaper);
    for (int i = 0; i < VL_DA_LENGTH(fonts->system_fonts); i++) {
        vl_font_search_description_deinit(fonts->system_fonts + i);
    }
    VL_DA_FREE(fonts->system_fonts);
    return VL_SUCCESS;
}