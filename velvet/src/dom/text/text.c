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
            // vl_css_value_print(parent_weight_value);
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

static void sort_web_font_candidates(vl_web_sized_font_t **fonts,
                                     int count, int target_weight) {
    for (int i = 1; i < count; i++) {
        vl_web_sized_font_t *candidate = fonts[i];
        int j = i;
        while (j > 0) {
            vl_web_sized_font_t *previous = fonts[j - 1];
            bool move_previous = false;
            if (!candidate) {
                move_previous = false;
            } else if (!previous) {
                move_previous = true;
            } else if (previous->priority > candidate->priority) {
                move_previous = true;
            } else if (previous->priority == candidate->priority &&
                       candidate->priority > 0 &&
                       VL_IABS(previous->weight - target_weight) <
                       VL_IABS(candidate->weight - target_weight)) {
                move_previous = true;
            }
            if (!move_previous) break;
            fonts[j] = previous;
            j--;
        }
        fonts[j] = candidate;
    }
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
    sort_web_font_candidates(blueprint->font_family,
                             VL_DA_LENGTH(blueprint->font_family),
                             blueprint->weight);
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
    if (VL_CSS_LAYOUT_NODE_IS_PSEUDO(element->layout)) {
        blueprint.text = element->layout.content_string;
    } else {
        blueprint.text = element->layout.span_ref_flatenned_text;
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
            try_add_web_family(web, font_families[i], &blueprint);
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
    if (element->layout.parent) {
        blueprint.compound_hash = vl_hash_combine(
            blueprint.compound_hash,
            vl_hash_bytes(&element->layout.parent->size.x, sizeof(float))
        );
    }

    blueprint.compound_hash = vl_hash_combine(
        blueprint.compound_hash,
        vl_hash_bytes(&element->layout.span_index, sizeof(element->layout.span_index))
    );
    if (element->layout.span_ref_region) {
        int span_count = VL_DA_LENGTH(element->layout.span_ref_region);
        blueprint.compound_hash = vl_hash_combine(
            blueprint.compound_hash,
            vl_hash_bytes(&span_count, sizeof(span_count))
        );
        const VL_DA(vl_css_layout_span_region_t) spans = element->layout.span_ref_region;
        for (int i = 0; i < span_count; i++) {
            blueprint.compound_hash = vl_hash_combine(
                blueprint.compound_hash,
                vl_hash_bytes((void *)&spans[i].len, sizeof(spans[i].len))
            );
            if (spans[i].len > 0 && spans[i].text) {
                blueprint.compound_hash = vl_hash_combine(
                    blueprint.compound_hash,
                    vl_hash_bytes((void*)spans[i].text, (size_t)spans[i].len)
                );
            }
        }
    }
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
    if (!lines) return NULL;
    vl_dom_element_text_line_t *line = VL_DA_PUSH(*lines, vl_dom_element_text_line_t);
    line->segments = VL_DA_INIT(vl_dom_element_text_segment_t, 4);
    return line;
}

static int glyph_owner_span(const vl_font_shaper_glyph_t *glyph) {
    if (!glyph) return -1;

    if (glyph->span_index >= 0)
        return glyph->span_index;

    if (glyph->span_cluster_first >= 0 &&
        glyph->span_cluster_last >= glyph->span_cluster_first)
        return glyph->span_cluster_first;

    return -1;
}

static void glyph_source_cluster_range(const vl_font_shaper_glyph_t *glyph,
                                       int *first, int *last) {
    int a = -1, b = -1;
    if (glyph) {
        if (glyph->source_cluster_first >= 0 &&
            glyph->source_cluster_last >= glyph->source_cluster_first) {
            a = glyph->source_cluster_first;
            b = glyph->source_cluster_last;
        } else if (glyph->codepoint_index >= 0) {
            a = b = glyph->codepoint_index;
        }
    }
    if (first) *first = a;
    if (last) *last = b;
}

static bool glyphs_share_source_cluster(const vl_font_shaper_glyph_t *a,
                                        const vl_font_shaper_glyph_t *b) {
    int af, al, bf, bl;
    glyph_source_cluster_range(a, &af, &al);
    glyph_source_cluster_range(b, &bf, &bl);
    return af >= 0 && bf >= 0 && af == bf && al == bl;
}

static void calculate_layout(vl_dom_element_t *element, vl_dom_element_text_layout_t *layout) {
    vl_web_t *web = element->owner->owner;
    vl_web_fonts_t *fonts = &web->fonts;

    element->layout.span_should_flip = false;
    element->layout.span_bidi_direction = VL_CSS_LAYOUT_BIDI_UNKNOWN;
    element->layout.span_paragraph_direction = VL_CSS_LAYOUT_BIDI_UNKNOWN;
    element->layout.span_bidi_level = -1;
    bool saw_rtl = false;
    bool saw_ltr = false;

    layout->lines = VL_DA_INIT(vl_dom_element_text_line_t);
    vl_font_shaper_run_t *run = vl_font_shaper_run_new(fonts->shaper);
    if (!run) return;

    vl_font_shaper_pop_all_fonts(fonts->shaper);
    for (int i = VL_DA_LENGTH(layout->blueprint.font_family); i --> 0;) {
        vl_web_sized_font_t *part = layout->blueprint.font_family[i];
        if (part && part->shaper_ref)
            vl_font_shaper_push_font(fonts->shaper, part->shaper_ref);
    }

    int span_index = element->layout.span_index;
    int span_count = VL_DA_LENGTH(element->layout.span_ref_region);
    VL_DA(vl_font_shaper_span_t) shaper_spans = VL_DA_INIT(vl_font_shaper_span_t);
    for (int i = 0; i < span_count; i++) {
        vl_font_shaper_span_t shaper_span = {
            .text = element->layout.span_ref_region[i].text,
            .len = element->layout.span_ref_region[i].len,
        };
        VL_DA_APPEND(shaper_spans, shaper_span);
    }
    vl_font_shaper_process_spans(fonts->shaper, shaper_spans, span_count,
                                    span_index < 0 ? 0 : span_index);
    VL_DA_FREE(shaper_spans);

    VL_DA(vl_font_segmentation_break_t) breaks = NULL;
    vl_font_segmentation_process_string(
        element->layout.span_ref_flatenned_text, VL_STRING_LEN(element->layout.span_ref_flatenned_text), 
        VL_FONT_SEGMENTATION_LINE, &breaks, VL_FONT_SEGMENTATION_INDEX_TYPE_CODEPOINT
    );

    vl_dom_element_text_line_t *line = push_new_line(&layout->lines);
    float base_x = element->layout.padding.w;
    float base_y = element->layout.padding.x;
    float segment_x = 0.0f, segment_y = 0.0f;
    float max_line_height = 0.0f, max_line_gap = 0.0f;
    vl_dom_element_text_segment_t segment = new_segment();
    bool wrap_allowed = element->layout.whitespace.text_wrap_mode == VL_CSS_LAYOUT_TEXT_WRAP;

    while (vl_font_shaper_shape(fonts->shaper, run)) {
        if (run->hard_line_break) {
            base_x = 0;
            float offset_amount = VL_MAX(max_line_height, run->font->ascent - run->font->descent - run->font->line_gap);
            base_y += offset_amount;
            line->height = VL_MAX(line->height, offset_amount);
            line->cursor_y = base_y;
            line = push_new_line(&layout->lines);
        }

        vl_font_info_t *sized_font = (vl_font_info_t*) run->font;
        if (!sized_font) continue;
        float line_height = sized_font->ascent - sized_font->descent;
        float span_position = element->layout.span_x_cursor;

        if (element->layout.span_paragraph_direction == VL_CSS_LAYOUT_BIDI_UNKNOWN) {
            if (run->paragraph_direction == VL_FONT_SHAPER_RUN_DIRECTION_RTL) 
                element->layout.span_paragraph_direction = VL_CSS_LAYOUT_BIDI_RTL;
            else if (run->paragraph_direction == VL_FONT_SHAPER_RUN_DIRECTION_LTR) 
                element->layout.span_paragraph_direction = VL_CSS_LAYOUT_BIDI_LTR;
        }

        VL_DA(vl_font_shaper_glyph_t) shaped_glyphs = VL_DA_INIT(vl_font_shaper_glyph_t);
        vl_font_shaper_glyph_t iter_glyph = {0};
        while (vl_font_shaper_iterate(run, &iter_glyph))
            VL_DA_APPEND(shaped_glyphs, iter_glyph);

        int shaped_glyph_count = VL_DA_LENGTH(shaped_glyphs);
        int cluster_begin = 0;
        while (cluster_begin < shaped_glyph_count) {
            int cluster_first = -1, cluster_last = -1;
            glyph_source_cluster_range(&shaped_glyphs[cluster_begin],
                                       &cluster_first, &cluster_last);
            int cluster_end = cluster_begin + 1;
            if (cluster_first >= 0 && cluster_last >= cluster_first) {
                while (cluster_end < shaped_glyph_count &&
                       glyphs_share_source_cluster(&shaped_glyphs[cluster_begin],
                                                   &shaped_glyphs[cluster_end]))
                    cluster_end++;
            }

            float cluster_origin_delta_x = 0.0f;
            bool spans_cross_inline_children =
                shaped_glyphs[cluster_begin].span_cluster_first >= 0 &&
                shaped_glyphs[cluster_begin].span_cluster_last >
                    shaped_glyphs[cluster_begin].span_cluster_first;
            if (span_index >= 0 && spans_cross_inline_children) {
                /* A glyph's pen position is measured from the beginning of the
                   complete shaping cluster, but this text node draws only the
                   glyphs owned by its span. Remove advances of glyphs that come
                   before this span in visual output order. RTL output runs in
                   reverse source-span order, so the comparison is reversed too. */
                for (int i = cluster_begin; i < cluster_end; i++) {
                    const vl_font_shaper_glyph_t *cluster_glyph = &shaped_glyphs[i];
                    int owner_span = glyph_owner_span(cluster_glyph);
                    bool precedes_this_span =
                        run->direction == VL_FONT_SHAPER_RUN_DIRECTION_LTR
                            ? owner_span >= 0 && owner_span < span_index
                            : run->direction == VL_FONT_SHAPER_RUN_DIRECTION_RTL
                                ? owner_span > span_index
                                : false;
                    if (precedes_this_span)
                        cluster_origin_delta_x += cluster_glyph->advance_x;
                }
            }

            float cluster_pen_x = 0.0f, cluster_pen_y = 0.0f;
            float cluster_owned_advance_x = 0.0f, cluster_owned_advance_y = 0.0f;
            for (int glyph_i = cluster_begin; glyph_i < cluster_end; glyph_i++) {
                vl_font_shaper_glyph_t shaper_glyph = shaped_glyphs[glyph_i];
                float glyph_advance_x = shaper_glyph.advance_x;
                bool cluster_end_glyph = glyph_i == cluster_end - 1;
                bool can_break = false;
                if (cluster_end_glyph && cluster_first >= 0 &&
                    cluster_last >= cluster_first) {
                    for (int i = 0; i < VL_DA_LENGTH(breaks); i++) {
                        bool boundary = run->direction == VL_FONT_SHAPER_RUN_DIRECTION_RTL
                            ? breaks[i].begin == (size_t)cluster_first
                            : breaks[i].end > 0 &&
                              breaks[i].end - 1 == (size_t)cluster_last;
                        if (boundary) {
                            can_break = true;
                            break;
                        }
                    }
                }

                bool glyph_owned = span_index < 0 ||
                    glyph_owner_span(&shaper_glyph) == span_index;
                bool glyph_cluster_contains_span = span_index >= 0 &&
                    shaper_glyph.span_cluster_first >= 0 &&
                    shaper_glyph.span_cluster_last >= shaper_glyph.span_cluster_first &&
                    shaper_glyph.span_cluster_first <= span_index &&
                    shaper_glyph.span_cluster_last >= span_index;
                bool finalize_segment = can_break || shaper_glyph.last;

                if (glyph_owned || glyph_cluster_contains_span) {
                    if (run->direction == VL_FONT_SHAPER_RUN_DIRECTION_RTL) saw_rtl = true;
                    else if (run->direction == VL_FONT_SHAPER_RUN_DIRECTION_LTR) saw_ltr = true;
                }

                if (glyph_owned) {
                    max_line_height = VL_MAX(line_height, max_line_height);
                    max_line_gap = VL_MAX(sized_font->line_gap, max_line_gap);
                    line->gap = VL_MAX(line->gap, sized_font->line_gap);

                    vl_dom_element_text_glyph_t text_glyph = {0};
                    text_glyph.id = shaper_glyph.id;
                    text_glyph.codepoint = shaper_glyph.codepoint;
                    text_glyph.font = sized_font->owner;
                    vl_web_font_atlas_codepoint_t web_codepoint = {0};
                    vl_web_fonts_find_glyph_id_with_font_and_font_index(
                            fonts, &web_codepoint, sized_font->owner,
                            shaper_glyph.id, sized_font->index);
                    vl_font_atlas_codepoint_t *atlas_codepoint =
                        fonts->atlases[web_codepoint.atlas_index].atlas.codepoints +
                        web_codepoint.codepoint_index;
                    text_glyph.uv = atlas_codepoint->uv;
                    text_glyph.brush = fonts->atlases[web_codepoint.atlas_index].brush;
                    float glyph_pen_x = segment_x - cluster_owned_advance_x +
                                        cluster_pen_x - cluster_origin_delta_x;
                    float glyph_pen_y = segment_y - cluster_owned_advance_y + cluster_pen_y;
                    text_glyph.x1 = glyph_pen_x + atlas_codepoint->x1 + shaper_glyph.x;
                    text_glyph.y1 = glyph_pen_y + atlas_codepoint->y1 - shaper_glyph.y;
                    text_glyph.x2 = glyph_pen_x + atlas_codepoint->x2 + shaper_glyph.x;
                    text_glyph.y2 = glyph_pen_y + atlas_codepoint->y2 - shaper_glyph.y;
                    VL_DA_APPEND(segment.glyphs, text_glyph);
                    segment.height = VL_MAX(segment.height, line_height);
                    segment.span_offset = VL_MAX(segment.span_offset, -sized_font->descent);
                    segment_x += glyph_advance_x;
                    segment_y += shaper_glyph.advance_y;
                    segment.width = VL_MAX(segment.width, segment_x);
                    cluster_owned_advance_x += glyph_advance_x;
                    cluster_owned_advance_y += shaper_glyph.advance_y;
                }

                cluster_pen_x += glyph_advance_x;
                cluster_pen_y += shaper_glyph.advance_y;

                if (finalize_segment) {
                    bool segment_has_content = VL_DA_LENGTH(segment.glyphs) > 0 ||
                        segment.height > 0 || segment_x != 0 || segment_y != 0;
                    if (segment_has_content) {
                        if (can_break && wrap_allowed &&
                            span_position + base_x + segment.width > element->layout.span_x_area &&
                            VL_DA_LENGTH(line->segments) > 0) {
                            line->wrapped = true;
                            base_x = 0;
                            base_y += max_line_height + max_line_gap;
                            if (span_position != 0 && element->parent) {
                                for (int i = 0; i < VL_DA_LENGTH(line->segments); i++)
                                    line->segments[i].x += span_position -
                                        element->parent->layout.effective_padding.w;
                            }
                            span_position = 0;
                            line = push_new_line(&layout->lines);
                            line->gap = sized_font->line_gap;
                            line->cursor_y = base_y;
                            max_line_height = 0;
                            max_line_gap = 0;
                        }
                        segment.x = base_x;
                        segment.y = base_y;
                        base_x += segment.width;
                        VL_DA_APPEND(line->segments, segment);
                        segment = new_segment();
                    }
                    segment_x = segment_y = 0;
                }
            }
            cluster_begin = cluster_end;
        }
        VL_DA_FREE(shaped_glyphs);
        element->layout.span_last_cursor = VL_VEC2(base_x, base_y);
    }

    if (saw_rtl && saw_ltr) element->layout.span_bidi_direction = VL_CSS_LAYOUT_BIDI_MIXED;
    else if (saw_rtl) element->layout.span_bidi_direction = VL_CSS_LAYOUT_BIDI_RTL;
    else if (saw_ltr) element->layout.span_bidi_direction = VL_CSS_LAYOUT_BIDI_LTR;
    element->layout.span_should_flip = (element->layout.span_bidi_direction == VL_CSS_LAYOUT_BIDI_RTL);

    VL_DA_FREE(segment.glyphs);
    vl_font_shaper_run_free(run);
    vl_font_shaper_pop_all_fonts(fonts->shaper);

    if (element->layout.parent && element->layout.parent->size.x >= 0) {
        float min_offset = VL_FLOAT_MAX;
        float width = element->layout.parent->size.x;
        for (int i = 0; i < VL_DA_LENGTH(layout->lines); i++) {
            vl_dom_element_text_line_t *line = layout->lines + i;
            for (int j = 0; j < VL_DA_LENGTH(line->segments); j++) {
                vl_dom_element_text_segment_t *segment = line->segments + j;
                line->width = VL_MAX(line->width,
                                     segment->x + segment->width);
                line->height = VL_MAX(line->height, segment->height);
                line->span_offset = VL_MAX(line->span_offset, segment->span_offset);
            }
            for (int j = 0; j < VL_DA_LENGTH(line->segments); j++) {
                vl_dom_element_text_segment_t *line_segment = line->segments + j;
                line_segment->y += (line->height - line_segment->height) -
                    (line->span_offset - line_segment->span_offset) + line->gap;
            }
            float align_offset = 0;
            if (layout->blueprint.alignment == VL_DOM_TEXT_ALIGN_CENTER)
                align_offset = width / 2 - line->width / 2;
            else if (layout->blueprint.alignment == VL_DOM_TEXT_ALIGN_END)
                align_offset = width - line->width;
            min_offset = VL_MIN(min_offset,
                                align_offset - element->layout.effective_padding.y / 2);
        }
        if (min_offset != VL_FLOAT_MAX && min_offset > 0)
            element->layout.position.x += min_offset;
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
    if (!element->layout.span_text || !text->layout.lines) return VL_ERROR;
    vl_dom_t *owner = element->owner;
    vl_web_t *web = owner->owner;
    // prepare_layout(element);
    float base_x = 0;
    float base_y = 0;
    // printf("%f %f %f\n", normalized_color.r, normalized_color.g, normalized_color.b);
    vl_dom_element_text_layout_t *layout = &text->layout;
    vl_quad_colors_t quad_color = VL_QUAD_COLOR(element->layout.color);
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
        VL_STRING_FREE(element->layout.span_text);
        element->layout.span_text = VL_STRING_INIT(value);
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
            size.y = VL_MAX(size.y, line->cursor_y + line->height + (i == VL_DA_LENGTH(layout->lines) - 1) * line->gap);
            element->layout.span_y_offset = line->span_offset;
            element->layout.span_line_height = line->height;
            element->layout.span_line_gap = line->gap;
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
    deinit_layout(&text->layout);
    vl_free(VL_DOM_ELEMENT_FUNCS(element));
    return VL_SUCCESS;
}