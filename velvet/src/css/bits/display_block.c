#include "support/da.h"
#include "support/result.h"
#include "velvet/css/layout.h"
#include "velvet/web/web.h"
#include "velvet/html/tags.h"

#include "apply_position.c"

struct vl_css_block_line_;
typedef struct vl_css_block_line_ vl_css_block_line;

static vl_css_block_line *push_new_block_line(VL_DA(vl_css_block_line) *lines);
static bool margins_can_collapse(vl_css_layout_node_t *prev, vl_css_layout_node_t *child);
static VL_DA(vl_css_block_line) layout_generic_div_ex(vl_css_layout_node_t *node);
static vl_result_t layout_generic_div(vl_css_layout_node_t *node);

#ifndef VELVET_CSS_BITS_DISPLAY_BLOCK_C
#define VELVET_CSS_BITS_DISPLAY_BLOCK_C

struct vl_css_block_line_ {
    VL_DA(vl_css_layout_node_t*) elements;
    float width, height;
    float max_span_offset;
};

static vl_css_block_line *push_new_block_line(VL_DA(vl_css_block_line) *lines) {
    if (!*lines) {
        *lines = VL_DA_INIT(vl_css_block_line);
    }
    vl_css_block_line *line = VL_DA_PUSH(*lines, vl_css_block_line);
    *line = (vl_css_block_line) {VL_DA_INIT(vl_css_layout_node_t*), 0, 0};
    return line;
}
#define PUSH_NEW_BLOCK_LINE(lines) push_new_block_line(&(lines))

static bool margins_can_collapse(vl_css_layout_node_t *prev, vl_css_layout_node_t *child) {
    if (!prev || !child) return false;
    return prev->padding.z == 0 && child->padding.x == 0;
}

static vl_result_t process_block_child(vl_css_layout_node_t *parent, vl_css_layout_node_t *child) {
    parent->size = VL_VEC2_SUB(parent->size, 
        VL_VEC2(parent->effective_padding.w + parent->effective_padding.y, parent->effective_padding.x + parent->effective_padding.z)
    );
    vl_css_layout_node_refresh_style(child);
    parent->size = VL_VEC2_ADD(parent->size, 
        VL_VEC2(parent->effective_padding.w + parent->effective_padding.y, parent->effective_padding.x + parent->effective_padding.z)
    );
    if (child->display == VL_CSS_LAYOUT_DISPLAY_NONE || child->position_type == VL_CSS_LAYOUT_POSITION_ABSOLUTE) {
        return VL_ERROR;
    }
    return VL_SUCCESS;
}

static void layout_pad_aware_node(vl_css_layout_node_t *parent, vl_css_layout_node_t *child) {
    parent->size = VL_VEC2_SUB(parent->size, 
        VL_VEC2(parent->effective_padding.w + parent->effective_padding.y, parent->effective_padding.x + parent->effective_padding.z)
    );
    vl_css_layout_node_layout(child);
    apply_position(parent);
    parent->size = VL_VEC2_ADD(parent->size, 
        VL_VEC2(parent->effective_padding.w + parent->effective_padding.y, parent->effective_padding.x + parent->effective_padding.z)
    );
}

static vl_css_layout_node_t *seek_next_child(vl_css_layout_node_t *parent, int *index) {
    (*index)++;
    int len = VL_DA_LENGTH(parent->children);
    if (*index >= len) return NULL;
    vl_css_layout_node_t *child = parent->children[*index];
    while (child && *index < len) {
        if (!process_block_child(parent, child)) return child;
        child = parent->children[++(*index)];
    }
    return NULL;
}

static VL_DA(vl_css_block_line) layout_generic_div_ex(vl_css_layout_node_t *node) {
    vl_vec2_t cursor = {node->effective_padding.w, node->effective_padding.x};
    bool lock_width = node->lock_dimensions[0];
    bool lock_height = node->lock_dimensions[1];
    if (node->raw_dimensions.x == VL_FLOAT_MIN) {
        if (node->display == VL_CSS_LAYOUT_DISPLAY_BLOCK)
            node->size.x = node->parent->size.x - node->margin.y - node->margin.w;
    } else {
        node->size.x = node->raw_dimensions.x;
    }
    if (node->raw_dimensions.y == VL_FLOAT_MIN) {
        node->size.y = 0;
    } else {
        node->size.y = node->raw_dimensions.y;
    }
    node->size.x = VL_MAX(node->size.x, node->effective_padding.w + node->effective_padding.y + lock_width * node->raw_dimensions.x);
    node->size.y = VL_MAX(node->size.y, node->effective_padding.x + node->effective_padding.z + lock_height * node->raw_dimensions.y);
    vl_vec2_t size = node->size;
    vl_vec2_t orig_size = size;
    VL_DA(vl_css_block_line) lines = VL_DA_INIT(vl_css_block_line);
    bool is_html = (strcmp(node->tag, "html") == 0);
    PUSH_NEW_BLOCK_LINE(lines);
    int i = -1;
    vl_css_layout_node_t *child = seek_next_child(node, &i);
    vl_css_layout_node_t *prev = NULL, *next = seek_next_child(node, &i);
    while (child) {
        vl_css_block_line *line = VL_DA_LAST(lines);
        if (node->block_first_margin == VL_FLOAT_MIN) {
            node->block_first_margin = VL_MAX(child->margin.x, child->block_first_margin);
            if (is_html) {
                size.y += child->margin.x;
            }
        }
        if (child->display == VL_CSS_LAYOUT_DISPLAY_BLOCK) cursor.x = node->effective_padding.w;
        child->position.x += cursor.x;
        child->span_x_cursor = node->position.x + cursor.x;
        child->span_x_area = node->parent->size.x;
        layout_pad_aware_node(node, child);

        if (child->display == VL_CSS_LAYOUT_DISPLAY_BLOCK) {
            if (!prev && (node->padding.x != 0)) {
                if (!is_html) cursor.y += child->margin.x;
                size.y += child->margin.x;
                child->block_applied_margin = child->margin.x;
            } else if (prev) {
                // if (VL_CSS_VALUE_COMPARE_LITERALS(prev->display, "inline")) {
                //     cursor.y += prev->size.y;
                //     line = PUSH_NEW_BLOCK_LINE(lines);
                // }
                if (margins_can_collapse(prev, child))  {
                    child->block_applied_margin = VL_MAX(child->margin.x, node->block_last_margin);
                    cursor.y += child->block_applied_margin;
                } else {
                    cursor.y += VL_MAX(VL_MAX(prev->block_last_margin, node->block_last_margin), child->margin.x);
                }
            }
            child->position.y += cursor.y - child->margin.x + (strcmp(node->tag, "html") == 0) * child->margin.x;
            if (node->allow_width_growth) size.x = VL_MAX(size.x, child->size.x + child->position.x + child->margin.y);
            size.x = VL_MIN(size.x, node->parent->size.x);
            if (!lock_height) size.y += (prev ? 1 : 0) * (VL_MAX(node->block_last_margin, child->margin.x)) + child->size.y;
            if (!next && (node->padding.z != 0 || is_html)) {
                size.y += child->margin.z;
            }
            cursor.y += child->size.y;
            line->width = VL_MAX(line->width, cursor.x + child->size.x);
            line->height = VL_MAX(line->height, child->size.y);
            VL_DA_APPEND(line->elements, child);
            if (next && next->display != VL_CSS_LAYOUT_DISPLAY_INLINE) {
                line = PUSH_NEW_BLOCK_LINE(lines);
            }
        } else {
            if (strcmp(child->tag, "br") == 0) {
                cursor.x = node->effective_padding.w;
                cursor.y += node->span_line_height;
                goto next;
            }
            if (prev && prev->display == VL_CSS_LAYOUT_DISPLAY_BLOCK) {
                line = PUSH_NEW_BLOCK_LINE(lines);
                if (margins_can_collapse(prev, child))  {
                    child->block_applied_margin = VL_MAX(child->margin.x, node->block_last_margin);
                    cursor.y += child->block_applied_margin;
                } else {
                    cursor.y += VL_MAX(VL_MAX(prev->block_last_margin, node->block_last_margin), child->margin.x);
                }
            } else if (!prev && (node->padding.x != 0)) {
                child->block_applied_margin = child->margin.x;
                cursor.y += child->margin.x;
                size.y += child->margin.x;
            }
            if (child->span_wrapped) {
                line = PUSH_NEW_BLOCK_LINE(lines);
            }
            child->position.y += cursor.y - child->margin.x;
            cursor.x += child->size.x + child->margin.y + child->margin.w;
            if (child->span_wrapped) {
                child->position.x = node->effective_padding.w;
                child->size.x = child->span_x_area - node->effective_padding.w - node->effective_padding.y;
            }
            node->span_wrapped = child->span_wrapped;
            line->width = VL_MAX(line->width, cursor.x);
            line->height = VL_MAX(line->height, child->size.y);
            size.x = VL_MAX(size.x, cursor.x);
            if (!node->allow_width_growth && !node->parent->allow_width_growth) size.x = VL_MIN(size.x, orig_size.x);
            // if (!lock_height && old_line_height < line->height) size.y += line->height - old_line_height;
            if (!lock_height && cursor.y + line->height + node->effective_padding.z > size.y) {
                size.y += cursor.y + line->height + node->effective_padding.z - size.y;
            }
            line->max_span_offset = VL_MAX(line->max_span_offset, child->span_y_offset);
            node->span_y_offset = VL_MAX(node->span_y_offset, child->span_y_offset);
            node->span_line_height = child->span_line_height;
            node->span_last_cursor = child->span_last_cursor;
            VL_DA_APPEND(line->elements, child);
            if (child->span_wrapped) {
                cursor.x = child->span_last_cursor.x + node->effective_padding.w;
                cursor.y += child->span_last_cursor.y;
                line = PUSH_NEW_BLOCK_LINE(lines);
            }
            if ((!child->span_wrapped && cursor.x > node->size.x && next) || (next && next->display == VL_CSS_LAYOUT_DISPLAY_BLOCK)) {
                cursor.x = node->effective_padding.w;
                cursor.y += line->height;
                line = PUSH_NEW_BLOCK_LINE(lines);
            }
        }
        next:
        if (!prev) {
            node->block_first_offset = child->padding.x + node->padding.x + node->parent->padding.x;
        }
        node->block_last_margin = VL_MAX(child->margin.z, child->block_last_margin);
        node->size = size;
        prev = child;
        child = next;
        next = seek_next_child(node, &i);
    }

    for (int i = 0; i < VL_DA_LENGTH(lines); i++) {
        vl_css_block_line *line = lines + i;
        float max_span_offset = line->max_span_offset;
        for (int j = 0; j < VL_DA_LENGTH(line->elements); j++) {
            vl_css_layout_node_t *child = line->elements[j];
            child->position.y += line->height - child->size.y + child->span_y_offset - max_span_offset;
            if (child->block_first_margin > child->block_applied_margin && child->block_applied_margin != VL_FLOAT_MIN && child->block_first_margin != VL_FLOAT_MIN) {
                for (int k = i; k < VL_DA_LENGTH(lines); k++) {
                    vl_css_block_line *line = lines + k;
                    for (int l = j; l < VL_DA_LENGTH(line->elements); l++) {
                        line->elements[l]->position.y += child->block_first_margin - child->block_applied_margin;
                    }
                }
            }
        }
    }
    return lines;
}

static vl_result_t layout_generic_div(vl_css_layout_node_t *node) {
    VL_DA(vl_css_block_line) lines = layout_generic_div_ex(node);
    for (int i = 0; i < VL_DA_LENGTH(lines); i++) {
        VL_DA_FREE(lines[i].elements);
    }
    VL_DA_FREE(lines);
    return VL_SUCCESS;
}

#endif // VELVET_CSS_BITS_DISPLAY_BLOCK_C