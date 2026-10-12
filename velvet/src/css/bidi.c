/* 
    bidi.c - small implementation of the unicode bidirectional algorithm
    proudly (actually not) implemented by chatgpt because i didn't have time to
    integrate gnu fribidi into velvet, things may change in the future
*/

#include "velvet/css/bidi.h"
#include "velvet/vendor/grapheme.h"
#include "support/base_math.h"
#include "support/da.h"
#include "support/memory.h"

#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

struct vl_css_bidi_context {
    const vl_css_layout_span_region_t *spans;
    size_t span_count;
    size_t codepoint_count;
    size_t *span_cp_start;
    size_t *span_cp_end;
    uint_least32_t *source;
    uint_least32_t *processed;
    uint_least32_t *line_processed;
    size_t *line_source_index;
    int_least8_t *levels;
    int_least8_t *line_levels;
    int_least8_t *working_levels;
    bool *line_spans;
    vl_css_layout_bidi_direction_t paragraph_direction;
    bool valid;
};

static void *bidi_alloc_array(size_t count, size_t item_size) {
    if (item_size == 0 || count > SIZE_MAX / item_size) return NULL;
    if (count == 0) count = 1;
    return vl_malloc(count * item_size);
}

static vl_css_layout_bidi_direction_t bidi_direction_from_grapheme(
    enum grapheme_bidirectional_direction direction) {
    if (direction == GRAPHEME_BIDIRECTIONAL_DIRECTION_RTL)
        return VL_CSS_LAYOUT_BIDI_RTL;
    if (direction == GRAPHEME_BIDIRECTIONAL_DIRECTION_LTR)
        return VL_CSS_LAYOUT_BIDI_LTR;
    return VL_CSS_LAYOUT_BIDI_UNKNOWN;
}

static enum grapheme_bidirectional_direction bidi_paragraph_override(
    vl_css_layout_node_t *node) {
    vl_css_value_t value = vl_css_layout_node_get_property(
        node, "direction", VL_CSS_VALUE_CONST_LITERAL("ltr"));
    if (VL_CSS_VALUE_COMPARE_LITERALS(value, "rtl"))
        return GRAPHEME_BIDIRECTIONAL_DIRECTION_RTL;
    if (VL_CSS_VALUE_COMPARE_LITERALS(value, "ltr"))
        return GRAPHEME_BIDIRECTIONAL_DIRECTION_LTR;
    return GRAPHEME_BIDIRECTIONAL_DIRECTION_NEUTRAL;
}

static void bidi_context_free_arrays(vl_css_bidi_context_t *context) {
    if (!context) return;
    vl_free(context->span_cp_start);
    vl_free(context->span_cp_end);
    vl_free(context->source);
    vl_free(context->processed);
    vl_free(context->line_processed);
    vl_free(context->line_source_index);
    vl_free(context->levels);
    vl_free(context->line_levels);
    vl_free(context->working_levels);
    vl_free(context->line_spans);
    context->span_cp_start = NULL;
    context->span_cp_end = NULL;
    context->source = NULL;
    context->processed = NULL;
    context->line_processed = NULL;
    context->line_source_index = NULL;
    context->levels = NULL;
    context->line_levels = NULL;
    context->working_levels = NULL;
    context->line_spans = NULL;
}

vl_css_bidi_context_t *vl_css_bidi_context_create(vl_css_layout_node_t *node) {
    if (!node) return NULL;
    vl_css_bidi_context_t *context = bidi_alloc_array(1, sizeof(*context));
    if (!context) return NULL;
    memset(context, 0, sizeof(*context));
    context->paragraph_direction = VL_CSS_LAYOUT_BIDI_UNKNOWN;

    // if (node->spans) {
    //     context->spans = node->spans;
    //     context->span_count = (size_t)VL_DA_LENGTH(node->spans);
    // } else 
    if (node->span_ref_region) {
        context->spans = node->span_ref_region;
        context->span_count = (size_t)VL_DA_LENGTH(node->span_ref_region);
    }
    if (!context->spans || context->span_count == 0) return context;

    size_t total_bytes = 0;
    for (size_t i = 0; i < context->span_count; i++) {
        const vl_css_layout_span_region_t *span = context->spans + i;
        if (span->len < 0 || (span->len > 0 && !span->text)) goto fail;
        if ((size_t)span->len > SIZE_MAX - total_bytes) goto fail;
        total_bytes += (size_t)span->len;
    }
    if (total_bytes == 0) return context;

    context->span_cp_start = bidi_alloc_array(context->span_count, sizeof(size_t));
    context->span_cp_end = bidi_alloc_array(context->span_count, sizeof(size_t));
    context->source = bidi_alloc_array(total_bytes, sizeof(uint_least32_t));
    context->processed = bidi_alloc_array(total_bytes, sizeof(uint_least32_t));
    context->line_processed = bidi_alloc_array(total_bytes, sizeof(uint_least32_t));
    context->line_source_index = bidi_alloc_array(total_bytes, sizeof(size_t));
    context->levels = bidi_alloc_array(total_bytes, sizeof(int_least8_t));
    context->line_levels = bidi_alloc_array(total_bytes, sizeof(int_least8_t));
    context->working_levels = bidi_alloc_array(total_bytes, sizeof(int_least8_t));
    context->line_spans = bidi_alloc_array(context->span_count, sizeof(bool));
    if (!context->span_cp_start || !context->span_cp_end || !context->source ||
        !context->processed || !context->line_processed ||
        !context->line_source_index || !context->levels || !context->line_levels ||
        !context->working_levels || !context->line_spans)
        goto fail;

    size_t cp_count = 0;
    for (size_t i = 0; i < context->span_count; i++) {
        const vl_css_layout_span_region_t *span = context->spans + i;
        context->span_cp_start[i] = cp_count;
        size_t byte_offset = 0;
        size_t byte_length = (size_t)span->len;
        while (byte_offset < byte_length) {
            uint_least32_t codepoint = GRAPHEME_INVALID_CODEPOINT;
            size_t consumed = grapheme_decode_utf8(
                span->text + byte_offset, byte_length - byte_offset, &codepoint);
            if (consumed == 0 || consumed > byte_length - byte_offset) {
                consumed = 1;
                codepoint = GRAPHEME_INVALID_CODEPOINT;
            }
            if (cp_count >= total_bytes) goto fail;
            context->source[cp_count++] = codepoint;
            byte_offset += consumed;
        }
        context->span_cp_end[i] = cp_count;
    }
    context->codepoint_count = cp_count;
    if (cp_count == 0) return context;

    enum grapheme_bidirectional_direction override = bidi_paragraph_override(node);
    size_t paragraph_offset = 0;
    while (paragraph_offset < cp_count) {
        enum grapheme_bidirectional_direction resolved =
            GRAPHEME_BIDIRECTIONAL_DIRECTION_NEUTRAL;
        size_t remaining = cp_count - paragraph_offset;
        size_t consumed = grapheme_bidirectional_preprocess_paragraph(
            context->source + paragraph_offset, remaining, override,
            context->processed + paragraph_offset, remaining, &resolved);
        if (consumed == 0 || consumed > remaining) goto fail;
        size_t level_count = grapheme_bidirectional_get_line_embedding_levels(
            context->processed + paragraph_offset, consumed,
            context->levels + paragraph_offset, consumed);
        if (level_count != consumed) goto fail;
        if (paragraph_offset == 0)
            context->paragraph_direction = bidi_direction_from_grapheme(resolved);
        paragraph_offset += consumed;
    }
    if (context->paragraph_direction == VL_CSS_LAYOUT_BIDI_UNKNOWN)
        context->paragraph_direction = VL_CSS_LAYOUT_BIDI_LTR;
    context->valid = true;
    return context;

fail:
    bidi_context_free_arrays(context);
    context->valid = false;
    return context;
}

void vl_css_bidi_context_destroy(vl_css_bidi_context_t *context) {
    if (!context) return;
    bidi_context_free_arrays(context);
    vl_free(context);
}

void vl_css_bidi_reset_node(vl_css_layout_node_t *node) {
    if (!node) return;
    node->span_should_flip = false;
    node->span_bidi_direction = VL_CSS_LAYOUT_BIDI_UNKNOWN;
    node->span_paragraph_direction = VL_CSS_LAYOUT_BIDI_UNKNOWN;
    node->span_bidi_level = -1;
}

void vl_css_bidi_accumulate_child(vl_css_layout_node_t *parent,
                                  const vl_css_layout_node_t *child) {
    if (!parent || !child || child->display != VL_CSS_LAYOUT_DISPLAY_INLINE ||
        (child->tag && strcmp(child->tag, "br") == 0))
        return;

    if (parent->span_paragraph_direction == VL_CSS_LAYOUT_BIDI_UNKNOWN &&
        child->span_paragraph_direction != VL_CSS_LAYOUT_BIDI_UNKNOWN)
        parent->span_paragraph_direction = child->span_paragraph_direction;

    vl_css_layout_bidi_direction_t child_direction = child->span_bidi_direction;
    if (child_direction == VL_CSS_LAYOUT_BIDI_UNKNOWN) return;
    if (child_direction == VL_CSS_LAYOUT_BIDI_MIXED) {
        parent->span_bidi_direction = VL_CSS_LAYOUT_BIDI_MIXED;
        return;
    }
    if (parent->span_bidi_direction == VL_CSS_LAYOUT_BIDI_UNKNOWN)
        parent->span_bidi_direction = child_direction;
    else if (parent->span_bidi_direction != VL_CSS_LAYOUT_BIDI_MIXED &&
             parent->span_bidi_direction != child_direction)
        parent->span_bidi_direction = VL_CSS_LAYOUT_BIDI_MIXED;
}

void vl_css_bidi_finalize_node(vl_css_layout_node_t *node,
                               const vl_css_bidi_context_t *context) {
    if (!node) return;
    if (context && context->valid)
        node->span_paragraph_direction = context->paragraph_direction;
    if (node->span_paragraph_direction == VL_CSS_LAYOUT_BIDI_UNKNOWN)
        node->span_paragraph_direction = VL_CSS_LAYOUT_BIDI_LTR;
    node->span_should_flip = node->span_bidi_direction == VL_CSS_LAYOUT_BIDI_RTL;
}

static bool bidi_child_is_reorderable(const vl_css_layout_node_t *child) {
    return child && child->display == VL_CSS_LAYOUT_DISPLAY_INLINE &&
        !child->span_wrapped && (!child->tag || strcmp(child->tag, "br") != 0);
}

static int bidi_fallback_direction(vl_css_layout_node_t **elements,
                                   int start, int end, int at,
                                   int base_direction) {
    vl_css_layout_node_t *child = elements[at];
    if (child->span_bidi_direction == VL_CSS_LAYOUT_BIDI_LTR ||
        child->span_bidi_direction == VL_CSS_LAYOUT_BIDI_RTL)
        return child->span_bidi_direction;
    if (child->span_bidi_direction == VL_CSS_LAYOUT_BIDI_MIXED)
        return base_direction;

    int before = VL_CSS_LAYOUT_BIDI_UNKNOWN;
    int after = VL_CSS_LAYOUT_BIDI_UNKNOWN;
    for (int i = at - 1; i >= start; i--) {
        int direction = elements[i]->span_bidi_direction;
        if (direction == VL_CSS_LAYOUT_BIDI_LTR || direction == VL_CSS_LAYOUT_BIDI_RTL) {
            before = direction;
            break;
        }
        if (direction == VL_CSS_LAYOUT_BIDI_MIXED) {
            before = base_direction;
            break;
        }
    }
    for (int i = at + 1; i < end; i++) {
        int direction = elements[i]->span_bidi_direction;
        if (direction == VL_CSS_LAYOUT_BIDI_LTR || direction == VL_CSS_LAYOUT_BIDI_RTL) {
            after = direction;
            break;
        }
        if (direction == VL_CSS_LAYOUT_BIDI_MIXED) {
            after = base_direction;
            break;
        }
    }
    return before != VL_CSS_LAYOUT_BIDI_UNKNOWN && before == after
        ? before : base_direction;
}

static void bidi_apply_line_l1(vl_css_bidi_context_t *context,
                               vl_css_layout_node_t **elements, int element_count) {
    if (!context || !context->valid || context->codepoint_count == 0) return;
    size_t cp_count = context->codepoint_count;
    memcpy(context->working_levels, context->levels,
           cp_count * sizeof(*context->working_levels));
    memset(context->line_spans, 0, context->span_count * sizeof(*context->line_spans));

    for (int i = 0; i < element_count; i++) {
        vl_css_layout_node_t *child = elements[i];
        if (!bidi_child_is_reorderable(child) || child->span_index < 0) continue;
        int first = child->span_index;
        int last = child->span_index_last >= first ? child->span_index_last : first;
        if ((size_t)first >= context->span_count) continue;
        if ((size_t)last >= context->span_count) last = (int)context->span_count - 1;
        for (int span = first; span <= last; span++)
            context->line_spans[span] = true;
    }

    size_t line_count = 0;
    for (size_t span = 0; span < context->span_count; span++) {
        if (!context->line_spans[span]) continue;
        size_t first = context->span_cp_start[span];
        size_t end = context->span_cp_end[span];
        if (end > cp_count) end = cp_count;
        for (size_t cp = first; cp < end; cp++) {
            if (line_count >= cp_count) return;
            context->line_processed[line_count] = context->processed[cp];
            context->line_source_index[line_count] = cp;
            line_count++;
        }
    }
    if (line_count == 0) return;
    size_t levels = grapheme_bidirectional_get_line_embedding_levels(
        context->line_processed, line_count, context->line_levels, line_count);
    if (levels != line_count) return;
    for (size_t i = 0; i < line_count; i++) {
        size_t source_index = context->line_source_index[i];
        if (source_index < cp_count)
            context->working_levels[source_index] = context->line_levels[i];
    }
}

static int bidi_child_source_level(const vl_css_bidi_context_t *context,
                                   const vl_css_layout_node_t *child,
                                   int base_level) {
    if (!context || !context->valid || !child || child->span_index < 0 ||
        (size_t)child->span_index >= context->span_count)
        return -1;

    int first_span = child->span_index;
    int last_span = child->span_index_last >= first_span
        ? child->span_index_last : first_span;
    if ((size_t)last_span >= context->span_count)
        last_span = (int)context->span_count - 1;

    size_t counts[126] = {0};
    int first_level = -1;
    size_t best_count = 0;
    for (int span = first_span; span <= last_span; span++) {
        size_t first_cp = context->span_cp_start[span];
        size_t end_cp = context->span_cp_end[span];
        if (end_cp > context->codepoint_count) end_cp = context->codepoint_count;
        for (size_t cp = first_cp; cp < end_cp; cp++) {
            int level = context->working_levels[cp];
            if (level < 0 || level >= (int)(sizeof(counts) / sizeof(counts[0])))
                continue;
            if (first_level < 0) first_level = level;
            size_t count = ++counts[level];
            if (count > best_count) best_count = count;
        }
    }
    if (first_level < 0) return -1;

    int best_level = first_level;
    best_count = counts[best_level];
    for (int level = 0; level < (int)(sizeof(counts) / sizeof(counts[0])); level++) {
        if (counts[level] > best_count) {
            best_count = counts[level];
            best_level = level;
        }
    }
    return best_level;
}

static float bidi_child_visual_width(const vl_css_layout_node_t *child) {
    if (!child) return 0.0f;
    if (child->span_wrapped && child->span_last_cursor.x >= 0.0f &&
        child->span_last_cursor.x < child->size.x)
        return child->span_last_cursor.x;
    return child->size.x;
}

static void bidi_flip_range(vl_css_layout_node_t **elements, int start, int end) {
    if (!elements || start < 0 || end <= start) return;
    float min_x = FLT_MAX;
    float max_x = -FLT_MAX;
    for (int i = start; i < end; i++) {
        vl_css_layout_node_t *child = elements[i];
        float width = bidi_child_visual_width(child);
        min_x = VL_MIN(min_x, child->position.x);
        max_x = VL_MAX(max_x, child->position.x + width);
    }
    if (!(min_x <= max_x)) return;
    for (int i = start; i < end; i++) {
        vl_css_layout_node_t *child = elements[i];
        float width = bidi_child_visual_width(child);
        child->position.x = min_x + max_x - (child->position.x + width);
    }
}

void vl_css_bidi_reorder_inline_line(vl_css_layout_node_t *node,
                                     vl_css_bidi_context_t *context,
                                     vl_css_layout_node_t **elements,
                                     int element_count) {
    if (!node || !elements || element_count <= 0) return;

    int base_direction = context && context->valid
        ? context->paragraph_direction : node->span_paragraph_direction;
    if (base_direction != VL_CSS_LAYOUT_BIDI_RTL)
        base_direction = VL_CSS_LAYOUT_BIDI_LTR;
    int base_level = base_direction == VL_CSS_LAYOUT_BIDI_RTL ? 1 : 0;

    bidi_apply_line_l1(context, elements, element_count);

    int i = 0;
    while (i < element_count) {
        vl_css_layout_node_t *child = elements[i];
        if (!bidi_child_is_reorderable(child)) {
            child->span_bidi_level = -1;
            i++;
            continue;
        }
        int start = i;
        while (i < element_count && bidi_child_is_reorderable(elements[i])) i++;
        int end = i;
        int max_level = base_level;
        int lowest_odd_level = INT_MAX;

        for (int j = start; j < end; j++) {
            child = elements[j];
            int level = bidi_child_source_level(context, child, base_level);
            if (level < 0) {
                int direction = bidi_fallback_direction(elements, start, end, j, base_direction);
                level = direction == VL_CSS_LAYOUT_BIDI_RTL ? 1 :
                    (base_level ? (direction == VL_CSS_LAYOUT_BIDI_LTR ? 2 : 1) : 0);
            }
            child->span_bidi_level = level;
            max_level = VL_MAX(max_level, level);
            if ((level & 1) && level < lowest_odd_level) lowest_odd_level = level;
        }

        /* Apply UAX #9 L2 to this inline fragment sequence. Embedding levels are
           computed per source codepoint by the bundled Unicode bidi algorithm;
           each indivisible inline box uses the dominant level in its span range. */
        if (lowest_odd_level != INT_MAX) {
            for (int level = max_level; level >= lowest_odd_level; level--) {
                int j = start;
                while (j < end) {
                    while (j < end && elements[j]->span_bidi_level < level) j++;
                    int range_start = j;
                    while (j < end && elements[j]->span_bidi_level >= level) j++;
                    if (j - range_start > 1)
                        bidi_flip_range(elements, range_start, j);
                }
            }
        }
    }

    /* CSS text-align:start follows the paragraph's resolved base direction. */
    if (node->display == VL_CSS_LAYOUT_DISPLAY_BLOCK &&
        base_direction == VL_CSS_LAYOUT_BIDI_RTL && node->size.x > 0) {
        vl_css_value_t align = vl_css_layout_node_get_property(
            node, "text-align", VL_CSS_VALUE_CONST_LITERAL("start"));
        if (VL_CSS_VALUE_COMPARE_LITERALS(align, "start")) {
            float max_x = -FLT_MAX;
            bool has_inline = false;
            for (int j = 0; j < element_count; j++) {
                vl_css_layout_node_t *child = elements[j];
                if (!bidi_child_is_reorderable(child)) continue;
                has_inline = true;
                max_x = VL_MAX(max_x, child->position.x + bidi_child_visual_width(child));
            }
            if (has_inline) {
                float offset = node->size.x - node->effective_padding.y - max_x;
                for (int j = 0; j < element_count; j++) {
                    vl_css_layout_node_t *child = elements[j];
                    if (bidi_child_is_reorderable(child)) child->position.x += offset;
                }
            }
        }
    }
}
