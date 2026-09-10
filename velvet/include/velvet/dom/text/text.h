#ifndef VELVET_DOM_ELEMENT_TEXT_H
#define VELVET_DOM_ELEMENT_TEXT_H

#include "font/atlas.h"
#include "graphics/brush.h"
#include "support/math.h"
#include "velvet/support/hash.h"
#include "velvet/support/result.h"
#include "velvet/dom/element.h"
#include "velvet/support/memory.h"
#include "velvet/support/da.h"
#include "web/fonts.h"
#include <stdio.h>

enum vl_dom_element_text_alignment {
    VL_DOM_TEXT_ALIGN_START = 1,
    VL_DOM_TEXT_ALIGN_CENTER,
    VL_DOM_TEXT_ALIGN_END,
    VL_DOM_TEXT_ALIGN_JUSTIFY
};
typedef enum vl_dom_element_text_alignment vl_dom_element_text_alignment_t;

struct vl_dom_element_text_blueprint {
    int height;
    int weight;
    VL_DA(VL_DA(vl_web_sized_font_t*)) font_family;
    vl_hash_t font_family_hash;
    vl_hash_t text_hash;
    vl_dom_element_text_alignment_t alignment;
};
typedef struct vl_dom_element_text_blueprint vl_dom_element_text_blueprint_t;

struct vl_dom_element_text_glyph {
    float x1, y1;
    float x2, y2;
    int32_t glyph_id, codepoint;
    bool line_break;
    vl_font_t *font;
    vl_graphics_brush_t *brush;
    vl_quad_uv_t uv;
};
typedef struct vl_dom_element_text_glyph vl_dom_element_text_glyph_t;

struct vl_dom_element_text_line {
    VL_DA(vl_dom_element_text_glyph_t) glyphs;
    float width, height, span_offset;
};
typedef struct vl_dom_element_text_line vl_dom_element_text_line_t;

struct vl_dom_element_text_layout {
    VL_DA(vl_dom_element_text_line_t) lines;
    vl_dom_element_text_blueprint_t blueprint;
};
typedef struct vl_dom_element_text_layout vl_dom_element_text_layout_t;

struct vl_dom_element_text {
    vl_dom_element_t base;
    VL_DA(char) text;
    vl_dom_element_text_layout_t layout;
};

typedef struct vl_dom_element_text vl_dom_element_text_t;

vl_dom_element_t *vl_dom_element_text_new(const char *tag, vl_source_location_t loc);
vl_result_t vl_dom_element_text_render(vl_dom_element_t *element);
vl_result_t vl_dom_element_text_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value);
vl_vec2_t vl_dom_element_text_get_content_size(vl_dom_element_t *element);
vl_result_t vl_dom_element_text_free(vl_dom_element_t *element);

#endif // VELVET_DOM_ELEMENT_TEXT_H