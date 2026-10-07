#include "velvet/dom/text/text.h"
#include "css/layout.h"
#include "css/style.h"
#include "dom/dom.h"
#include "dom/element.h"
#include "font/atlas.h"
#include "font/font.h"
#include "font/segmentation.h"
#include "font/shaper.h"
#include "support/base_math.h"
#include "support/hash.h"
#include "support/ht.h"
#include "support/math.h"
#include "support/da.h"
#include "support/measurement.h"
#include "support/memory.h"
#include "support/result.h"
#include "support/managed_assert.h"
#include "vendor/utf8.h"
#include "web/fonts.h"
#include "web/web.h"

vl_dom_element_t *vl_dom_element_text_new(const char *tag, vl_source_location_t loc) {
    vl_dom_element_funcs_t *funcs = vl_malloc(sizeof(vl_dom_element_funcs_t) + sizeof(vl_dom_element_text_t));
    if (!funcs) return NULL;
    funcs->render = vl_dom_element_text_render;
    funcs->set_property = vl_dom_element_text_set_property;
    funcs->get_content_size = vl_dom_element_text_get_content_size;
    funcs->free = vl_dom_element_text_free;
    vl_dom_element_text_t *element = VL_PTR_FORWARD(funcs, sizeof(*funcs));
    element->base.tag = tag;
    return (vl_dom_element_t*) element;
}

static VL_DA(vl_web_sized_font_t*) try_get_web_font(vl_web_fonts_t *fonts, const char *family, int weight, int height) {
    VL_DA(vl_web_sized_font_t*) result = vl_web_fonts_get_font(
        fonts, family, weight, height
    );
    if (!result) {
        vl_web_fonts_add_family_from_system(fonts, family);
        result = vl_web_fonts_get_font(
            fonts, family, weight, height
        );
    }
    return result;
}

static int correct_weight(vl_web_fonts_t *fonts, const char *name, int weight) {
    if (!name) return weight;
    for (int i = 0; i < VL_DA_LENGTH(fonts->families); i++) {
        vl_web_font_family_t *family = fonts->families + i;
        if (utf8casecmp(family->name, name) != 0) continue;
        for (int j = 0; j < VL_DA_LENGTH(family->variations); j++) {
            vl_web_font_t *font = family->variations + j;
            if (font->weight == weight) return weight;
        }
        int diff = VL_INT_MAX;
        int res = weight;
        for (int j = 0; j < VL_DA_LENGTH(family->variations); j++) {
            vl_web_font_t *font = family->variations + j;
            int curr_diff = VL_IABS(font->weight - weight);
            if (curr_diff > diff) continue;
            diff = curr_diff;
            res = font->weight;
        }
        return res;
    }
    return weight;
}

// static const char *family_name_by_unit_font(vl_web_fonts_t *fonts, vl_font_t *unit_font) {
//     for (int i = 0; i < VL_DA_LENGTH(fonts->families); i++) {
//         vl_web_font_family_t *family = fonts->families + i;
//         for (int j = 0; j < VL_DA_LENGTH(family->variations); j++) {
//             vl_web_font_t *font = family->variations + j;
//             for (int k = 0; k < VL_DA_LENGTH(font->parts); k++) {
//                 vl_web_font_part_t *part = font->parts + k;
//                 if (part->unit_font == unit_font) return family->name;
//             }
//         }
//     }
//     return NULL;
// }

static int calculate_weight(vl_dom_element_t *element, vl_css_value_t weight) {
    if (!element) return 400;
    if (VL_CSS_VALUE_IS_LITERAL(weight)) {
        if (VL_CSS_VALUE_COMPARE_LITERALS(weight, "extralight")) return VL_WEB_FONT_EXTRA_LIGHT;
        if (VL_CSS_VALUE_COMPARE_LITERALS(weight, "light")) return VL_WEB_FONT_LIGHT;
        if (VL_CSS_VALUE_COMPARE_LITERALS(weight, "normal")) return VL_WEB_FONT_REGULAR;
        if (VL_CSS_VALUE_COMPARE_LITERALS(weight, "regular")) return VL_WEB_FONT_REGULAR;
        if (VL_CSS_VALUE_COMPARE_LITERALS(weight, "semibold")) return VL_WEB_FONT_SEMI_BOLD;
        if (VL_CSS_VALUE_COMPARE_LITERALS(weight, "bold")) return VL_WEB_FONT_BOLD;
        if (VL_CSS_VALUE_COMPARE_LITERALS(weight, "extrabold")) return VL_WEB_FONT_EXTRA_BOLD;
        if (VL_CSS_VALUE_COMPARE_LITERALS(weight, "black")) return VL_WEB_FONT_BLACK;
        if (VL_CSS_VALUE_COMPARE_LITERALS(weight, "extrablack")) return VL_WEB_FONT_EXTRA_BLACK;
        if (VL_CSS_VALUE_COMPARE_LITERALS(weight, "bolder") || VL_CSS_VALUE_COMPARE_LITERALS(weight, "lighter")) {
            vl_css_value_t parent_weight_value = vl_css_layout_node_get_property(element->layout.parent, "font-weight", VL_CSS_VALUE_INTEGER(400));
            int parent_weight = calculate_weight(element->parent, parent_weight_value);
            bool bolder = VL_CSS_VALUE_COMPARE_LITERALS(weight, "bolder");
            if (bolder) {
                if (parent_weight < 100) return 100;
                if (parent_weight >= 100 && parent_weight <= 300) return 400;
                if (parent_weight >= 400 && parent_weight <= 500) return 700;
                if (parent_weight >= 600 && parent_weight <= 700) return 900;
                if (parent_weight >= 800 && parent_weight <= 900) return 900;
                if (parent_weight > 900) return 900;
            } else {
                if (parent_weight < 100) return 100;
                if (parent_weight >= 100 && parent_weight <= 300) return 100;
                if (parent_weight >= 400 && parent_weight <= 500) return 100;
                if (parent_weight >= 600 && parent_weight <= 700) return 400;
                if (parent_weight >= 800 && parent_weight <= 900) return 400;
                if (parent_weight > 900) return 400;
            }
        }
    }
    if (weight.type == VL_CSS_VALUE_INTEGER) {
        return weight.as.integer;
    }
    return 700;
}

static int try_get_glyph_id(vl_dom_element_text_blueprint_t *blueprint, UChar32 codepoint) {
    for (int i = VL_DA_LENGTH(blueprint->font_family); i --> 0;) {
        vl_web_sized_font_t *font = blueprint->font_family[i];
        if (!font) continue;
        int glyph_id = vl_font_get_glyph_id_by_codepoint(font->font, codepoint);
        if (glyph_id != 0) return glyph_id;
    }
    return 0;
}

static VL_DA(vl_web_sized_font_t*) calculate_used_fonts(vl_dom_element_text_blueprint_t *blueprint) {
    VL_DA(vl_web_sized_font_t*) result = VL_DA_INIT(vl_web_sized_font_t*);
    UChar32 codepoint = 0;
    const char *text = blueprint->text;
    while (*text != '\0' && (text = utf8codepoint(text, &codepoint))) {
        int glyph_id = 0;
        vl_web_sized_font_t *font = NULL;
        for (int j = VL_DA_LENGTH(blueprint->font_family); j --> 0;) {
            font = blueprint->font_family[j];
            if (!font) continue;
            glyph_id = vl_font_get_glyph_id_by_codepoint(font->font, codepoint);
            if (glyph_id != 0) break;
        }
        if (glyph_id != 0 && font) {
            bool already_added = false;
            for (int j = 0; j < VL_DA_LENGTH(result); j++) {
                if (result[j] == font) {
                    already_added = true;
                    break;
                }
            }
            if (!already_added) VL_DA_APPEND(result, font);
        }
    }
    return result;
}

static void try_add_web_family(vl_web_t *web, const char *name, vl_dom_element_text_blueprint_t *blueprint) {
    vl_measurement_t measure = {0};
    vl_measurement_start(&measure, "font finding");
    VL_DA(vl_web_sized_font_t*) whole_family = try_get_web_font(&web->fonts, name, -1, blueprint->height);
    if (!whole_family) {
        whole_family = vl_web_fonts_get_font(&web->fonts, "serif", -1, blueprint->height);
    }
    for (int i = VL_DA_LENGTH(whole_family); i --> 0;) {
        vl_web_sized_font_t *part = whole_family[i];
        VL_DA_APPEND(blueprint->font_family, part);
    }
    int max_priority = 0;
    for (int i = 0; i < VL_DA_LENGTH(blueprint->font_family); i++) {
        vl_web_sized_font_t *a = blueprint->font_family[i];
        for (int j = 0; j < VL_DA_LENGTH(blueprint->font_family) - i; j++) {
            vl_web_sized_font_t *b = blueprint->font_family[i + j];
            if (a->priority > b->priority) {
                vl_web_sized_font_t tmp = *b;
                *b = *a;
                *a = tmp;
            }
            max_priority = VL_MAX(max_priority, b->priority);
        }
        max_priority = VL_MAX(max_priority, a->priority);
    }
    for (int priority = 1; priority <= max_priority; priority++) {
        int start = -1;
        int end = -1;
        for (int i = 0; i < VL_DA_LENGTH(blueprint->font_family); i++) {
            vl_web_sized_font_t *font = blueprint->font_family[i];
            if (font->priority == priority && start < 0) {
                start = i;
            }
            if ((font->priority != priority && start >= 0)) {
                end = i;
                break;
            }
        }
        if (end < 0) end = VL_DA_LENGTH(blueprint->font_family);
        if (start < 0) continue;
        for (int i = start; i < end; i++) {
            vl_web_sized_font_t *a = blueprint->font_family[i];
            for (int j = start; j < end; j++) {
                vl_web_sized_font_t *b = blueprint->font_family[j];
                if (VL_IABS(a->weight - blueprint->weight) > VL_IABS(b->weight - blueprint->weight)) {
                    vl_web_sized_font_t tmp = *b;
                    *b = *a;
                    *a = tmp;
                }
            }
        }
    }
    // for (int i = 0; i < VL_DA_LENGTH(blueprint->font_family); i++) {
    //     printf("%s %i\n", blueprint->font_family[i]->font->name, blueprint->font_family[i]->priority);
    // }
    VL_DA(vl_web_sized_font_t*) used_fonts = calculate_used_fonts(blueprint);
    VL_DA_FREE(blueprint->font_family);
    blueprint->font_family = used_fonts;
    VL_DA_FREE(whole_family);
    vl_measurement_end(&measure);
    // vl_measurement_print(&measure);
}

static vl_dom_element_text_blueprint_t calculate_blueprint(vl_dom_element_t *element, bool hollow) {
    vl_dom_element_text_blueprint_t blueprint = {0};
    vl_web_t *web = element->owner->owner;
    vl_css_value_t font_size_css = vl_css_layout_node_get_property(element->layout.parent ? element->layout.parent : &element->layout, "font-size", VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_EM(1)));
    vl_css_value_t font_weight_css = vl_css_layout_node_get_property(element->layout.parent, "font-weight", VL_CSS_VALUE_INTEGER(400));
    font_size_css.as.metric1 = vl_css_layout_node_process_metric(element->layout.parent ? element->layout.parent : &element->layout, "font-size", font_size_css.as.metric1, 0);
    blueprint.height = font_size_css.as.metric1.value;
    blueprint.weight = calculate_weight(element, font_weight_css);
    if (!VL_CSS_LAYOUT_NODE_IS_PSEUDO(element->layout)) {
        blueprint.text = ((vl_dom_element_text_t*) element)->text;
    } else {
        blueprint.text = element->layout.content_string;
    }

    vl_css_value_t text_align_css = vl_css_layout_node_get_property(&element->layout, "text-align", VL_CSS_VALUE_CONST_LITERAL("start"));
    if (VL_CSS_VALUE_COMPARE_LITERALS(text_align_css, "start")) {
        blueprint.alignment = VL_DOM_TEXT_ALIGN_START;
    } else if (VL_CSS_VALUE_COMPARE_LITERALS(text_align_css, "center")) {
        blueprint.alignment = VL_DOM_TEXT_ALIGN_CENTER;
    } else if (VL_CSS_VALUE_COMPARE_LITERALS(text_align_css, "end")) {
        blueprint.alignment = VL_DOM_TEXT_ALIGN_END;
    } else if (VL_CSS_VALUE_COMPARE_LITERALS(text_align_css, "justify")) {
        blueprint.alignment = VL_DOM_TEXT_ALIGN_JUSTIFY;
    }

    vl_css_value_t font_family = vl_css_layout_node_get_property(&element->layout, "font-family", VL_CSS_VALUE_CONST_LITERAL("serif"));
    if (!hollow) blueprint.font_family = VL_DA_INIT(vl_web_sized_font_t*);
    if (VL_CSS_VALUE_IS_LITERAL(font_family)) {
        if (!hollow) {
            try_add_web_family(web, font_family.as.literal, &blueprint);
        }
        blueprint.compound_hash = vl_hash_string(font_family.as.literal);
    } else if (font_family.type == VL_CSS_VALUE_FONT_LIST && font_family.as.font_list.fonts) {
        VL_DA(VL_STRING) font_families = font_family.as.font_list.fonts;
        for (int i = 0; i < VL_DA_LENGTH(font_families); i++) {
            if (!hollow) {
                *VL_DA_PUSH(blueprint.font_family, VL_DA(vl_web_sized_font_t*)) =
                try_get_web_font(&web->fonts, font_families[i], correct_weight(&web->fonts, font_families[i], blueprint.weight), blueprint.height);
            }
            blueprint.compound_hash = vl_hash_combine(
                blueprint.compound_hash,
                vl_hash_string(font_families[i])
            );
        }
    }
    blueprint.compound_hash = vl_hash_combine(
        blueprint.compound_hash,
        vl_hash_string(blueprint.text)
    );
    blueprint.compound_hash = vl_hash_combine(
        blueprint.compound_hash,
        vl_hash_bytes(&element->layout.parent->size.x, sizeof(float))
    );
    return blueprint;
}

static void deinit_blueprint(vl_dom_element_text_blueprint_t *blueprint) {
    VL_DA_FREE(blueprint->font_family);
    VL_ZERO_OUT(blueprint);
}

static vl_dom_element_text_segment_t new_segment() {
    return (vl_dom_element_text_segment_t) {
        .glyphs = VL_DA_INIT(vl_dom_element_text_glyph_t, 4),
        .x = 0, .y = 0,
        .width = 0, .height = 0, .span_offset = 0,
    };
}

static vl_dom_element_text_line_t *push_new_line(VL_DA(vl_dom_element_text_line_t) *lines) {
    vl_dom_element_text_line_t *line = VL_DA_PUSH(*lines, vl_dom_element_text_line_t);
    line->segments = VL_DA_INIT(vl_dom_element_text_segment_t, 4);
    return line;
}

static void calculate_layout(vl_dom_element_t *element, vl_dom_element_text_layout_t *layout) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    vl_web_t *web = element->owner->owner;
    vl_web_fonts_t *fonts = &web->fonts;
    layout->lines = VL_DA_INIT(vl_dom_element_text_line_t);
    vl_font_shaper_run_t *run = vl_font_shaper_run_new(fonts->shaper);
    vl_font_shaper_pop_all_fonts(fonts->shaper);
    for (int i = VL_DA_LENGTH(layout->blueprint.font_family); i --> 0;) {
        vl_web_sized_font_t* part = layout->blueprint.font_family[i];
        if (!part) continue;
        vl_font_shaper_push_font(fonts->shaper, part->shaper_ref);
        printf("pushing %s\n", part->font->name);
    }
    // printf("layout: '%s' %f %f\n", layout->blueprint.text, element->layout.span_position.x, element->layout.span_area.x);
    vl_font_shaper_process(fonts->shaper, layout->blueprint.text, VL_STRING_LEN(layout->blueprint.text));
    VL_DA(vl_font_segmentation_break_t) breaks = NULL;
    vl_font_segmentation_process_string(layout->blueprint.text, VL_STRING_LEN(layout->blueprint.text), VL_FONT_SEGMENTATION_LINE, &breaks, VL_FONT_SEGMENTATION_INDEX_TYPE_CODEPOINT);
    vl_dom_element_text_line_t *line = push_new_line(&layout->lines);
    float base_x = element->layout.padding.w;
    float base_y = element->layout.padding.x;
    float segment_x = 0;
    float segment_y = 0;
    vl_dom_element_text_segment_t segment = new_segment();
    float max_line_height = 0;
    while (vl_font_shaper_shape(fonts->shaper, run)) {
        if (run->hard_line_break) {
            base_x = 0;
            base_y += segment.height;
            line = push_new_line(&layout->lines);
        }
        vl_font_shaper_glyph_t shaper_glyph = {0};
        vl_font_info_t *sized_font = (vl_font_info_t*) run->font;
        // printf("%s %s %i %i %i %p %p %p\n", font_info->owner->name, sized_fat_font->font->name, font_info->owner->height, sized_fat_font->font->height, font_info->index, font_info, sized_fat_font, sized_font);
        if (!sized_font) continue;
        float line_height = sized_font->ascent - sized_font->descent + sized_font->line_gap;
        max_line_height = VL_MAX(line_height, max_line_height);
        float span_position = element->layout.span_x_cursor;
        while (vl_font_shaper_iterate(run, &shaper_glyph)) {
            bool can_break = false;
            for (int i = 0; i < VL_DA_LENGTH(breaks); i++) {
                if (breaks[i].end - 1 == shaper_glyph.codepoint_index) {
                    can_break = true;
                    break;
                }
            }
            vl_dom_element_text_glyph_t text_glyph = {0};
            text_glyph.id = shaper_glyph.id;
            text_glyph.codepoint = shaper_glyph.codepoint;
            text_glyph.font = sized_font->owner;
            vl_web_font_atlas_codepoint_t web_codepoint = {0};
            vl_web_fonts_find_glyph_id_with_font_and_font_index(&web->fonts, &web_codepoint, sized_font->owner, shaper_glyph.id, sized_font->index);
            vl_font_atlas_codepoint_t *atlas_codepoint = fonts->atlases[web_codepoint.atlas_index].atlas.codepoints + web_codepoint.codepoint_index;
            text_glyph.uv = atlas_codepoint->uv;
            text_glyph.brush = fonts->atlases[web_codepoint.atlas_index].brush;
            text_glyph.x1 = segment_x + atlas_codepoint->x1 + shaper_glyph.x;
            text_glyph.y1 = segment_y + atlas_codepoint->y1 - shaper_glyph.y;
            text_glyph.x2 = text_glyph.x1 + atlas_codepoint->w;
            text_glyph.y2 = text_glyph.y1 + atlas_codepoint->h;
            // printf("text_glyph.y1 = %c %f\n", shaper_glyph.codepoint, atlas_codepoint->y2 - sized_font->font->ascent);
            segment.width = VL_MAX(segment.width, segment_x + VL_MAX(atlas_codepoint->x2, shaper_glyph.advance_x));
            segment.height = VL_MAX(segment.height, VL_MAX(line_height, atlas_codepoint->y2));
            segment.span_offset = VL_MAX(segment.span_offset, -sized_font->descent);
            line->gap = VL_MAX(line->gap, sized_font->line_gap);
            segment_x += shaper_glyph.advance_x;
            segment_y += shaper_glyph.advance_y;
            VL_DA_APPEND(segment.glyphs, text_glyph);
            line->width = VL_MAX(line->width, base_x + segment_x);
            // printf("glyph id: %i advance x: %f\n", shaper_glyph.id, shaper_glyph.advance_x);
            if (can_break || shaper_glyph.last) {
                if (span_position + base_x + segment.width > element->layout.span_x_area && VL_DA_LENGTH(line->segments) > 0) {
                    // printf("breaking %c: %s\n", shaper_glyph.codepoint, layout->blueprint.text);
                    line->wrapped = true;
                    base_x = 0;
                    base_y += max_line_height;
                    for (int i = 0; i < VL_DA_LENGTH(line->segments); i++) {
                        line->segments[i].x += span_position;
                    }
                    span_position = 0;
                    line = push_new_line(&layout->lines);
                    max_line_height = 0;
                }
                segment.x = base_x;
                segment.y = base_y;
                segment_x = segment_y = 0;
                base_x += segment.width;
                element->layout.span_last_cursor.x = base_x;
                element->layout.span_last_cursor.y = base_y;
                VL_DA_APPEND(line->segments, segment);
                segment = new_segment();
            }
        }
    }
    VL_DA_FREE(segment.glyphs);
    element->layout.span_last_cursor = VL_VEC2(base_x, base_y);
    vl_font_shaper_run_free(run);
    vl_font_shaper_pop_all_fonts(fonts->shaper);

    // printf("layouting: %s %f\n", element->tag, element->layout.parent->size.x);
    if (element->layout.parent->size.x >= 0) {
        float min_offset = VL_FLOAT_MAX;
        float width = element->layout.parent->size.x;
        for (int i = 0; i < VL_DA_LENGTH(layout->lines); i++) {
            vl_dom_element_text_line_t *line = layout->lines + i;
            for (int j = 0; j < VL_DA_LENGTH(line->segments); j++) {
                vl_dom_element_text_segment_t *segment = line->segments + j;
                line->width = VL_MAX(line->width, segment->x + segment->width);
                line->height = VL_MAX(line->height, segment->height);
                line->span_offset = VL_MAX(line->span_offset, segment->span_offset);
                element->layout.span_y_offset = VL_MAX(element->layout.span_y_offset, line->span_offset);
            }
            for (int j = 0; j < VL_DA_LENGTH(line->segments); j++) {
                vl_dom_element_text_segment_t *segment = line->segments + j;
                segment->y += (line->height - segment->height) - (line->span_offset - segment->span_offset);
            }
            float align_offset = 0;
            if (element->layout.parent) {
                if (layout->blueprint.alignment == VL_DOM_TEXT_ALIGN_CENTER) {
                    align_offset = width / 2 - line->width / 2;
                } else if (layout->blueprint.alignment == VL_DOM_TEXT_ALIGN_END) {
                    align_offset = width - line->width;
                }
            }
            min_offset = VL_MIN(min_offset, align_offset - element->layout.effective_padding.y / 2);
        }
        if (min_offset != VL_FLOAT_MAX && min_offset > 0) element->layout.position.x += min_offset;
    } 
    VL_DA_FREE(breaks);
}

static void deinit_layout(vl_dom_element_text_layout_t *layout) {
    if (layout->lines) {
        for (int i = 0; i < VL_DA_LENGTH(layout->lines); i++) {
            vl_dom_element_text_line_t *line = layout->lines + i;
            for (int j = 0; j < VL_DA_LENGTH(line->segments); j++) {
                vl_dom_element_text_segment_t *segment = line->segments + j;
                VL_DA_FREE(segment->glyphs);
            }
            VL_DA_FREE(line->segments);
        }
    }
    VL_DA_FREE(layout->lines);
    deinit_blueprint(&layout->blueprint);
}

static void prepare_layout(vl_dom_element_t *element) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    vl_dom_element_text_blueprint_t *blueprint = &text->layout.blueprint;
    vl_dom_element_text_blueprint_t fresh_blueprint = calculate_blueprint(element, true);
    if (blueprint->height != fresh_blueprint.height 
            || blueprint->weight != fresh_blueprint.weight
            || blueprint->compound_hash != fresh_blueprint.compound_hash
            || blueprint->alignment != fresh_blueprint.alignment) {
        deinit_layout(&text->layout);
        text->layout.blueprint = calculate_blueprint(element, false);
        calculate_layout(element, &text->layout);
    }
}

vl_result_t vl_dom_element_text_render(vl_dom_element_t *element) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    if (!text->layout.blueprint.text) return VL_ERROR;
    vl_dom_t *owner = element->owner;
    vl_web_t *web = owner->owner;
    prepare_layout(element);
    float base_x = 0;
    float base_y = 0;
    vl_color_t normalized_color = VL_COLOR(
        element->layout.color.r,
        element->layout.color.g,
        element->layout.color.b,
        element->layout.color.a
    );
    // printf("%f %f %f\n", normalized_color.r, normalized_color.g, normalized_color.b);
    vl_dom_element_text_layout_t *layout = &text->layout;
    vl_quad_colors_t quad_color = VL_QUAD_COLOR(normalized_color);
    for (int i = 0; i < VL_DA_LENGTH(layout->lines); i++) {
        vl_dom_element_text_line_t *line = layout->lines + i;
        for (int j = 0; j < VL_DA_LENGTH(line->segments); j++) {
            vl_dom_element_text_segment_t *segment = line->segments + j;
            for (int k = 0; k < VL_DA_LENGTH(segment->glyphs); k++) {
                vl_dom_element_text_glyph_t *glyph = segment->glyphs + k;
                float x = segment->x;
                float y = segment->y;
                vl_graphics_render_batch_rect_colored_uv(web->render, VL_RECT_EX(
                    x + glyph->x1, y + glyph->y1,
                    x + glyph->x2, y + glyph->y2
                ), glyph->brush, quad_color, glyph->uv);
            }
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_dom_element_text_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    if (strcmp(property, "innerText") == 0) {
        if (type != VL_DOM_ELEMENT_PROPERTY_STRING) VL_ASSERT(0 && "innerText property requires a STRING");
        VL_STRING_FREE(text->text);
        text->text = VL_STRING_INIT(value);
        return VL_SUCCESS;
    }
    return VL_ERROR;
}

vl_vec2_t vl_dom_element_text_get_content_size(vl_dom_element_t *element) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    prepare_layout(element);
    vl_dom_element_text_layout_t *layout = &text->layout;
    vl_vec2_t size = {0};
    vl_web_t *web = element->owner->owner;
    if (layout->lines) {
        for (int i = 0; i < VL_DA_LENGTH(layout->lines); i++) {
            vl_dom_element_text_line_t *line = layout->lines + i;
            size.x = VL_MAX(line->width, size.x);
            if (VL_DA_LENGTH(line->segments) > 0) size.y += line->height;
            // printf("line: %s %f %zu\n", layout->blueprint.text, line->height, VL_DA_LENGTH(line->segments));
            element->layout.span_y_offset = line->span_offset - line->gap;
            element->layout.span_line_height = line->height;
            if (line->wrapped) element->layout.span_wrapped = true;
        }
    }
    // printf("span offset '%s' %f %f %f %i\n", text->text, element->layout.span_y_offset, size.x, size.y, element->layout.span_wrapped);
    size.x += element->layout.padding.y;
    size.y += element->layout.padding.z;
    return size;
}

vl_result_t vl_dom_element_text_free(vl_dom_element_t *element) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    VL_STRING_FREE(text->text);
    deinit_layout(&text->layout);
    vl_free(VL_DOM_ELEMENT_FUNCS(element));
    return VL_SUCCESS;
}