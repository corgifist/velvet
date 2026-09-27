#include "velvet/css/layout.h"
#include "velvet/web/web.h"
#include "velvet/html/tags.h"

#include "apply_position.c"

struct vl_css_block_line_;
typedef struct vl_css_block_line_ vl_css_block_line;

static vl_css_block_line *push_new_block_line(VL_DA(vl_css_block_line) *lines);
static bool margins_can_collapse(vl_css_layout_node_t *prev, vl_css_layout_node_t *child);
static void try_add_layout_target(VL_DA(vl_css_layout_node_t*) *targets, vl_css_layout_node_t *node);
static VL_DA(vl_css_layout_node_t*) get_layout_targets(vl_css_layout_node_t *node);
static VL_DA(vl_css_block_line) layout_generic_div_ex(vl_css_layout_node_t *node);
static vl_result_t layout_generic_div(vl_css_layout_node_t *node);

#ifndef VELVET_CSS_BITS_DISPLAY_BLOCK_C
#define VELVET_CSS_BITS_DISPLAY_BLOCK_C

struct vl_css_block_line_ {
    VL_DA(vl_css_layout_node_t*) elements;
    float width, height;
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

static void try_add_layout_target(VL_DA(vl_css_layout_node_t*) *targets, vl_css_layout_node_t *node) {
    vl_css_layout_node_process(node);
    apply_position(node);
    if (node->display == VL_CSS_LAYOUT_DISPLAY_NONE || node->position_type == VL_CSS_LAYOUT_POSITION_ABSOLUTE) {
        return;
    }
    VL_DA_APPEND(*targets, node);
}

static VL_DA(vl_css_layout_node_t*) get_layout_targets(vl_css_layout_node_t *node) {
    VL_DA(vl_css_layout_node_t*) layout_targets = VL_DA_INIT(vl_css_layout_node_t*);
    if (node->pseudo_before) try_add_layout_target(&layout_targets, node->pseudo_before);
    for (int i = 0; i < VL_DA_LENGTH(node->children); i++) {
        vl_css_layout_node_t *child = node->children[i];
        try_add_layout_target(&layout_targets, child);
    }
    if (node->pseudo_after) try_add_layout_target(&layout_targets, node->pseudo_after);
    return layout_targets;
}

static VL_DA(vl_css_block_line) layout_generic_div_ex(vl_css_layout_node_t *node) {
    vl_vec2_t cursor = {node->effective_padding.w, node->effective_padding.x};
    bool lock_width = node->lock_dimensions[0];
    bool lock_height = node->lock_dimensions[1];
    // printf("%s locks: %i (%f) %i (%f)\n", node->tag, lock_width, node->raw_dimensions.x, lock_height, node->raw_dimensions.y);
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
    node->size.x -= (node->effective_padding.w + node->effective_padding.y);
    node->size.y -= (node->effective_padding.x + node->effective_padding.z);
    VL_DA(vl_css_layout_node_t*) layout_targets = get_layout_targets(node);
    node->size.x += (node->effective_padding.w + node->effective_padding.y);
    node->size.y += (node->effective_padding.x + node->effective_padding.z);
    bool is_void_tag = vl_html_is_tag_void(node->tag);
    int len = VL_DA_LENGTH(layout_targets);
    VL_DA(vl_css_block_line) lines = VL_DA_INIT(vl_css_block_line);
    PUSH_NEW_BLOCK_LINE(lines);
    for (int i = 0; i < len; i++) {
        vl_css_layout_node_t *prev = i > 0 ? layout_targets[i - 1] : NULL;
        vl_css_layout_node_t *child = layout_targets[i];
        vl_css_layout_node_t *next = i < len - 1 ? layout_targets[i + 1] : NULL;
        vl_css_block_line *line = lines + VL_DA_LENGTH(lines) - 1;
        if (node->block_first_margin == VL_FLOAT_MIN) {
            node->block_first_margin = VL_MAX(child->margin.x, child->block_first_margin);
        }
        if (child->display == VL_CSS_LAYOUT_DISPLAY_BLOCK) {
            if (!prev && (node->padding.x != 0)) {
                cursor.y += child->margin.x;
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
                    cursor.y += prev->margin.z;
                }
            }
            cursor.x = node->effective_padding.w;
            child->position.x += cursor.x;
            child->position.y += cursor.y - child->margin.x;
            if (node->allow_width_growth) size.x = VL_MAX(size.x, child->size.x + child->position.x + child->margin.y);
            size.x = VL_MIN(size.x, node->parent->size.x);
            if (!lock_height) size.y += (prev ? 1 : 0) * (VL_MAX(node->block_last_margin, child->margin.x)) + child->size.y;
            if (!next && node->padding.z != 0) {
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
            if (prev && prev->display == VL_CSS_LAYOUT_DISPLAY_BLOCK) {
                line = PUSH_NEW_BLOCK_LINE(lines);
                if (margins_can_collapse(prev, child))  {
                    child->block_applied_margin = VL_MAX(child->margin.x, node->block_last_margin);
                    cursor.y += child->block_applied_margin;
                } else {
                    cursor.y += prev->margin.z;
                }
            } else if (!prev && (node->padding.x != 0)) {
                child->block_applied_margin = child->margin.x;
                cursor.y += child->margin.x;
                size.y += child->margin.x;
            }
            child->position.x += cursor.x;
            child->position.y += cursor.y - child->margin.x;
            cursor.x += child->size.x + child->margin.y + child->margin.w;
            line->width = VL_MAX(line->width, cursor.x);
            float old_line_height = line->height;
            line->height = VL_MAX(line->height, child->size.y);
            size.x = VL_MAX(size.x, cursor.x);
            if (!node->allow_width_growth && !node->parent->allow_width_growth) size.x = VL_MIN(size.x, node->parent->size.x - node->margin.y - node->margin.w);
            if (!lock_height && old_line_height < line->height) size.y += line->height - old_line_height;
            if (!lock_height && cursor.y + line->height + node->effective_padding.z > size.y) {
                size.y += cursor.y + line->height + node->effective_padding.z - size.y;
            }
            node->span_y_offset = VL_MAX(node->span_y_offset, child->span_y_offset);
            VL_DA_APPEND(line->elements, child);
            if ((cursor.x > node->size.x && next) || (next && next->display == VL_CSS_LAYOUT_DISPLAY_BLOCK)) {
                cursor.x = 0;
                cursor.y += line->height;
                line = PUSH_NEW_BLOCK_LINE(lines);
            }
        }
        node->block_last_margin = VL_MAX(child->margin.z, child->block_last_margin);
        node->size = size;
        // apply_position(child);
    }
    // printf("%s lines: %zu\n", node->tag, VL_DA_LENGTH(lines));
    for (int i = 0; i < VL_DA_LENGTH(lines); i++) {
        vl_css_block_line *line = lines + i;
        float max_span_offset = 0;
        for (int j = 0; j < VL_DA_LENGTH(line->elements); j++) {
            vl_css_layout_node_t *child = line->elements[j];
            max_span_offset = VL_MAX(max_span_offset, child->span_y_offset);
        }
        for (int j = 0; j < VL_DA_LENGTH(line->elements); j++) {
            vl_css_layout_node_t *child = line->elements[j];
            child->position.y += line->height - child->size.y - (max_span_offset - child->span_y_offset);
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
    VL_DA_FREE(layout_targets);
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