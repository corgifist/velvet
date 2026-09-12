#include "velvet/dom/text/text.h"
#include "css/layout.h"
#include "css/style.h"
#include "dom/dom.h"
#include "dom/element.h"
#include "font/atlas.h"
#include "support/base_math.h"
#include "support/hash.h"
#include "support/math.h"
#include "support/da.h"
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

static const char *family_name_by_unit_font(vl_web_fonts_t *fonts, vl_font_t *unit_font) {
    for (int i = 0; i < VL_DA_LENGTH(fonts->families); i++) {
        vl_web_font_family_t *family = fonts->families + i;
        for (int j = 0; j < VL_DA_LENGTH(family->variations); j++) {
            vl_web_font_t *font = family->variations + j;
            for (int k = 0; k < VL_DA_LENGTH(font->parts); k++) {
                vl_web_font_part_t *part = font->parts + k;
                if (part->unit_font == unit_font) return family->name;
            }
        }
    }
    return NULL;
}

static int calculate_weight(vl_css_value_t weight) {
    if (VL_CSS_VALUE_IS_LITERAL(weight)) {
        if (VL_CSS_CONST_LITERAL_EQUAL(weight, "normal")) return 400;
        if (VL_CSS_CONST_LITERAL_EQUAL(weight, "bold")) return 700;
    }
    if (weight.type == VL_CSS_VALUE_INTEGER) {
        return weight.as.integer;
    }
    return 400;
}

static vl_dom_element_text_blueprint_t calculate_blueprint(vl_dom_element_t *element, bool hollow) {
    vl_dom_element_text_blueprint_t blueprint = {0};
    vl_web_t *web = element->owner->owner;
    vl_css_value_t font_size_css = vl_css_layout_node_get_property(element->layout.parent ? element->layout.parent : &element->layout, "font-size", VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_EM(1)));
    vl_css_value_t font_weight_css = vl_css_layout_node_get_property(element->layout.parent, "font-weight", VL_CSS_VALUE_INTEGER(400));
    font_size_css.as.metric1 = vl_css_layout_node_process_metric(element->layout.parent ? element->layout.parent : &element->layout, "font-size", font_size_css.as.metric1, 0);
    blueprint.height = font_size_css.as.metric1.value;
    blueprint.weight = calculate_weight(font_weight_css);

    vl_css_value_t text_align_css = vl_css_layout_node_get_property(&element->layout, "text-align", VL_CSS_VALUE_CONST_LITERAL("start"));
    if (VL_CSS_CONST_LITERAL_EQUAL(text_align_css, "start")) {
        blueprint.alignment = VL_DOM_TEXT_ALIGN_START;
    } else if (VL_CSS_CONST_LITERAL_EQUAL(text_align_css, "center")) {
        blueprint.alignment = VL_DOM_TEXT_ALIGN_CENTER;
    } else if (VL_CSS_CONST_LITERAL_EQUAL(text_align_css, "end")) {
        blueprint.alignment = VL_DOM_TEXT_ALIGN_END;
    } else if (VL_CSS_CONST_LITERAL_EQUAL(text_align_css, "justify")) {
        blueprint.alignment = VL_DOM_TEXT_ALIGN_JUSTIFY;
    }

    vl_css_value_t font_family = vl_css_layout_node_get_property(&element->layout, "font-family", VL_CSS_VALUE_CONST_LITERAL("serif"));
    if (!hollow) blueprint.font_family = VL_DA_INIT(VL_DA(vl_web_sized_font_t*));
    if (VL_CSS_VALUE_IS_LITERAL(font_family)) {
        if (!hollow) *VL_DA_PUSH(blueprint.font_family, VL_DA(vl_web_sized_font_t*)) = 
            try_get_web_font(&web->fonts, font_family.as.literal, correct_weight(&web->fonts, font_family.as.literal, blueprint.weight), blueprint.height);
        blueprint.font_family_hash = vl_hash_string(font_family.as.literal);
    } else if (font_family.type == VL_CSS_VALUE_FONT_LIST && font_family.as.font_list.fonts) {
        VL_DA(VL_DA_STRING) font_families = font_family.as.font_list.fonts;
        for (int i = 0; i < VL_DA_LENGTH(font_families); i++) {
            if (!hollow) *VL_DA_PUSH(blueprint.font_family, VL_DA(vl_web_sized_font_t*)) =
                try_get_web_font(&web->fonts, font_families[i], correct_weight(&web->fonts, font_families[i], blueprint.weight), blueprint.height);
            blueprint.font_family_hash = vl_hash_combine(
                blueprint.font_family_hash,
                vl_hash_string(font_families[i])
            );
        }
    }
    blueprint.text_hash = vl_hash_string(((vl_dom_element_text_t*) element)->text);
    return blueprint;
}

static void deinit_blueprint(vl_dom_element_text_blueprint_t *blueprint) {
    if (blueprint->font_family) {
        for (int i = 0; i < VL_DA_LENGTH(blueprint->font_family); i++) {
            VL_DA_FREE(blueprint->font_family[i]);
        }
    }
    VL_DA_FREE(blueprint->font_family);
    VL_ZERO_OUT(blueprint);
}

static vl_dom_element_text_line_t *push_new_line(VL_DA(vl_dom_element_text_line_t) *lines) {
    vl_dom_element_text_line_t *line = VL_DA_PUSH(*lines, vl_dom_element_text_line_t);
    line->glyphs = VL_DA_INIT(vl_dom_element_text_glyph_t);
    return line;
}

static void calculate_layout(vl_dom_element_t *element, vl_dom_element_text_layout_t *layout) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    vl_web_t *web = element->owner->owner;
    vl_web_fonts_t *fonts = &web->fonts;
    layout->lines = VL_DA_INIT(vl_dom_element_text_line_t);
    vl_font_shaper_run_t *run = vl_font_shaper_run_new(fonts->shaper);
    for (int i = 0; i < VL_DA_LENGTH(layout->blueprint.font_family); i++) {
        VL_DA(vl_web_sized_font_t*) parts = layout->blueprint.font_family[i];
        if (!parts) continue;
        for (int j = 0; j < VL_DA_LENGTH(parts); j++) {
            vl_font_shaper_push_font(fonts->shaper, parts[j]->shaper_ref);
        }
    }
    vl_font_shaper_process(fonts->shaper, text->text, VL_DA_LENGTH(text->text) - 1);
    // printf("shaped '%s'\n", text->text);
    vl_dom_element_text_line_t *line = push_new_line(&layout->lines);
    float base_x = 0;
    float base_y = 0;
    while (vl_font_shaper_shape(fonts->shaper, run)) {
        if (run->hard_line_break) {
            line = push_new_line(&layout->lines);
        }
        vl_font_shaper_glyph_t shaper_glyph = {0};
        int corrected_weight = correct_weight(fonts, family_name_by_unit_font(fonts, run->font), layout->blueprint.weight);
        vl_web_sized_font_t *sized_font = vl_web_fonts_get_font_by_unit_font(fonts, run->font, corrected_weight, layout->blueprint.height);
        if (!sized_font) continue;
        while (vl_font_shaper_iterate(run, &shaper_glyph)) {
            vl_dom_element_text_glyph_t text_glyph = {0};
            text_glyph.glyph_id = shaper_glyph.id;
            text_glyph.codepoint = shaper_glyph.codepoint;
            text_glyph.font = sized_font->font;
            vl_web_font_atlas_codepoint_t web_codepoint = {0};
            vl_web_fonts_find_glyph_id_with_font(&web->fonts, &web_codepoint, sized_font->font, text_glyph.glyph_id);
            vl_font_atlas_codepoint_t *atlas_codepoint = fonts->atlases[web_codepoint.atlas_index].atlas.codepoints + web_codepoint.codepoint_index;
            text_glyph.uv = atlas_codepoint->uv;
            text_glyph.brush = fonts->atlases[web_codepoint.atlas_index].brush;
            text_glyph.x1 = base_x + atlas_codepoint->x1 + shaper_glyph.x * layout->blueprint.height;
            text_glyph.y1 = base_y + atlas_codepoint->y1 - shaper_glyph.y * layout->blueprint.height;
            text_glyph.x2 = text_glyph.x1 + atlas_codepoint->w;
            text_glyph.y2 = text_glyph.y1 + atlas_codepoint->h;
            line->width = VL_MAX(line->width, text_glyph.x2 + (text_glyph.codepoint == ' ' ? shaper_glyph.advance_x * layout->blueprint.height : 0));
            line->height = VL_MAX(line->height, text_glyph.y2);
            line->span_offset = VL_MAX(line->span_offset, atlas_codepoint->y2 - text_glyph.font->ascent);
            base_x += shaper_glyph.advance_x * layout->blueprint.height;
            base_y += shaper_glyph.advance_y * layout->blueprint.height;
            VL_DA_APPEND(line->glyphs, text_glyph);
        }
    }
    vl_font_shaper_run_free(run);
    vl_font_shaper_pop_all_fonts(fonts->shaper);

    // printf("layouting: %s %f\n", element->tag, element->layout.parent->size.x);
    if (element->layout.parent->size.x >= 0) {
        float min_offset = VL_FLOAT_MAX;
        for (int i = 0; i < VL_DA_LENGTH(layout->lines); i++) {
            vl_dom_element_text_line_t *line = layout->lines + i;
            float align_offset = 0;
            if (element->layout.parent) {
                if (layout->blueprint.alignment == VL_DOM_TEXT_ALIGN_CENTER) {
                    align_offset = element->layout.parent->size.x / 2 - line->width / 2;
                } else if (layout->blueprint.alignment == VL_DOM_TEXT_ALIGN_END) {
                    align_offset = element->layout.parent->size.x - line->width;
                }
            }
            min_offset = VL_MIN(min_offset, align_offset);
        }
        if (min_offset != VL_FLOAT_MAX) element->layout.position.x += min_offset;
    } 
}

static void deinit_layout(vl_dom_element_text_layout_t *layout) {
    if (layout->lines) {
        for (int i = 0; i < VL_DA_LENGTH(layout->lines); i++) {
            VL_DA_FREE(layout->lines[i]);
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
            || blueprint->font_family_hash != fresh_blueprint.font_family_hash
            || blueprint->text_hash != fresh_blueprint.text_hash
            || blueprint->alignment != fresh_blueprint.alignment) {
        deinit_layout(&text->layout);
        text->layout.blueprint = calculate_blueprint(element, false);
        calculate_layout(element, &text->layout);
    }
}

vl_result_t vl_dom_element_text_render(vl_dom_element_t *element) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    if (!text->text) return VL_ERROR;
    vl_dom_t *owner = element->owner;
    vl_web_t *web = owner->owner;
    prepare_layout(element);
    float base_x = 0;
    float base_y = 0;
    vl_css_value_t text_color = vl_css_layout_node_get_property(&element->layout, "color", VL_CSS_VALUE_RGBA(0, 0, 0, 1));
    vl_color_t normalized_color = VL_COLOR(
        text_color.as.rgba.r / 255.0f,
        text_color.as.rgba.g / 255.0f,
        text_color.as.rgba.b / 255.0f,
        text_color.as.rgba.a
    );
    vl_dom_element_text_layout_t *layout = &text->layout;
    vl_quad_colors_t quad_color = VL_QUAD_COLOR(normalized_color);
    for (int i = 0; i < VL_DA_LENGTH(layout->lines); i++) {
        vl_dom_element_text_line_t *line = layout->lines + i;
        for (int j = 0; j < VL_DA_LENGTH(line->glyphs); j++) {
            vl_dom_element_text_glyph_t *glyph = line->glyphs + j;
            vl_graphics_render_batch_rect_colored_uv(web->render, VL_RECT_EX(
                glyph->x1, glyph->y1,
                glyph->x2, glyph->y2
            ), glyph->brush, quad_color, glyph->uv);
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_dom_element_text_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    if (strcmp(property, "innerText") == 0) {
        if (type != VL_DOM_ELEMENT_PROPERTY_STRING) VL_ASSERT(0 && "innerText property requires a STRING");
        if (text->text) VL_DA_FREE(text->text);
        text->text = VL_DA_INIT_FROM_STRING(value);
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
            size.y += line->height;
            element->layout.span_y_offset = VL_MAX(element->layout.span_y_offset, line->span_offset);
        }
    }
    return size;
}

vl_result_t vl_dom_element_text_free(vl_dom_element_t *element) {
    vl_dom_element_text_t *text = (vl_dom_element_text_t*) element;
    VL_DA_FREE(text->text);
    deinit_layout(&text->layout);
    vl_free(VL_DOM_ELEMENT_FUNCS(element));
    return VL_SUCCESS;
}