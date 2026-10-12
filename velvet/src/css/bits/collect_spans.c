#include "velvet/css/layout.h"
#include "velvet/support/base_math.h"
#include <string.h>

static void recurse_spans(vl_css_layout_node_t *node);
static void span_collect_children(vl_css_layout_node_t *root, vl_css_layout_node_t *parent, VL_DA(vl_css_layout_span_region_t) *regions);
static void span_flush_run(vl_css_layout_node_t *root, VL_DA(vl_css_layout_span_region_t) *regions);

#ifndef VELVET_CSS_BITS_COLLECT_SPANS_C
#define VELVET_CSS_BITS_COLLECT_SPANS_C

#include "update_children_style.c"

static VL_STRING flatten_spans(VL_DA(vl_css_layout_span_region_t) spans) {
    VL_DA(char) result = VL_DA_INIT(char);

    for (int i = 0; i < VL_DA_LENGTH(spans); i++) {
        vl_css_layout_span_region_t *span = spans + i;

        for (int j = 0; j < span->len; j++)
            *VL_DA_PUSH(result, char) = span->text[j];
    }

    VL_STRING flattened = VL_STRING_FROM_DA(result);
    printf("flattened result: '%s'\n", flattened);
    VL_DA_FREE(result);
    return flattened;
}

static bool span_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

static void span_collapse_whitespace(VL_DA(vl_css_layout_span_region_t) regions) {
    // trim leading whitespaces
    for (int i = 0; i < VL_DA_LENGTH(regions); i++) {
        vl_css_layout_span_region_t *region = regions + i;
        int original_len = region->len;
        int source_index = 0;
        while (source_index < original_len &&
               span_is_whitespace(region->text[source_index])) {
            source_index++;
        }

        bool region_is_all_whitespace = source_index == original_len;
        if (source_index > 0) {
            VL_STRING original_text = region->text;
            region->text = VL_STRING_INIT(original_text + source_index);
            region->len = original_len - source_index;
            VL_STRING_FREE(original_text);
        }

        if (!region_is_all_whitespace) break;
    }

    // trim trailing whitespaces
    for (int i = VL_DA_LENGTH(regions); i-- > 0;) {
        vl_css_layout_span_region_t *region = regions + i;
        int source_index = region->len - 1;
        while (source_index >= 0 &&
               span_is_whitespace(region->text[source_index])) {
            source_index--;
        }

        int new_len = source_index + 1;
        bool region_is_all_whitespace = new_len == 0;
        if (new_len != region->len) {
            VL_STRING original_text = region->text;
            region->text = VL_STRING_INIT(original_text, new_len);
            region->len = new_len;
            VL_STRING_FREE(original_text);
        }

        if (!region_is_all_whitespace) break;
    }

    // collapse spaces mid-text
    bool saw_space = false;
    for (int i = 0; i < VL_DA_LENGTH(regions); i++) {
        vl_css_layout_span_region_t *region = regions + i;
        VL_DA(char) new_text = VL_DA_INIT(char);
        for (int source_offset = 0; source_offset < region->len; source_offset++) {
            char c = region->text[source_offset];
            if (span_is_whitespace(c)) {
                if (!saw_space) {
                    *VL_DA_PUSH(new_text, char) = ' ';
                    saw_space = true;
                }
            } else {
                *VL_DA_PUSH(new_text, char) = c;
                saw_space = false;
            }
        }

        VL_STRING original_text = region->text;
        region->text = VL_STRING_FROM_DA(new_text);
        region->len = VL_DA_LENGTH(new_text);
        VL_STRING_FREE(original_text);
        VL_DA_FREE(new_text);
    }
}

static bool span_is_tag(const vl_css_layout_node_t *node, const char *tag) {
    return node && node->tag && strcmp(node->tag, tag) == 0;
}

static void span_reset_node(vl_css_layout_node_t *node) {
    node->span_index = node->span_index_last = -1;
    node->span_ref_region = NULL;
    node->span_ref_flatenned_text = NULL;
    VL_STRING_FREE(node->span_flattened_text);
}

static void span_bind_node(vl_css_layout_node_t *node,
                           VL_DA(vl_css_layout_span_region_t) run,
                           VL_STRING flattened, int index) {
    if (node->span_index == -2) return;

    if (!node->span_ref_region) {
        node->span_index = node->span_index_last = index;
        node->span_ref_region = run;
        node->span_ref_flatenned_text = flattened;
    } else if (node->span_ref_region == run) {
        node->span_index = VL_MIN(node->span_index, index);
        node->span_index_last = VL_MAX(node->span_index_last, index);
    } else {
        node->span_index = node->span_index_last = -2;
        node->span_ref_region = NULL;
        node->span_ref_flatenned_text = NULL;
    }
}

static void span_flush_run(vl_css_layout_node_t *root,
                           VL_DA(vl_css_layout_span_region_t) *regions) {
    if (!regions || !*regions) return;

    if (VL_DA_EMPTY(*regions)) {
        VL_DA_FREE(*regions);
        return;
    }

    VL_DA(vl_css_layout_span_region_t) run = *regions;
    *regions = NULL;

    VL_DA(vl_css_layout_span_region_t) *slot = VL_DA_PUSH(root->span_regions, VL_DA(vl_css_layout_span_region_t));

    if (!slot) {
        VL_DA_FREE(run);
        return;
    }

    *slot = run;

    span_collapse_whitespace(run);
    VL_STRING flattened = flatten_spans(run);
    if (run[0].owner)
        run[0].owner->span_flattened_text = flattened;
    else
        VL_STRING_FREE(flattened);

    for (int i = 0; i < VL_DA_LENGTH(run); i++) {
        vl_css_layout_node_t *owner = run[i].owner;
        if (!owner) continue;

        span_bind_node(owner, run, flattened, i);

        for (vl_css_layout_node_t *ancestor = owner->parent;
             ancestor && ancestor != root;
             ancestor = ancestor->parent) {
            if (ancestor->display == VL_CSS_LAYOUT_DISPLAY_BLOCK) break;

            if (ancestor->display == VL_CSS_LAYOUT_DISPLAY_INLINE)
                span_bind_node(ancestor, run, flattened, i);
        }
    }
}

static void span_collect_child(vl_css_layout_node_t *root,
                               vl_css_layout_node_t *parent,
                               vl_css_layout_node_t *child,
                               VL_DA(vl_css_layout_span_region_t) *regions) {
    if (!child || update_children_style(parent, child) != VL_SUCCESS)
        return;

    if (span_is_tag(child, "text")) {
        span_reset_node(child);
        if (!child->span_text) return;

        if (!*regions)
            *regions = VL_DA_INIT(vl_css_layout_span_region_t);
        if (!*regions) return;

        vl_css_layout_span_region_t region = {
            .owner = child,
            .text = VL_STRING_COPY(child->span_text),
            .len = VL_STRING_LEN(child->span_text)
        };

        if (!region.text) return;
        VL_DA_APPEND(*regions, region);
        return;
    }

    if (span_is_tag(child, "br")) {
        span_reset_node(child);
        span_flush_run(root, regions);
        return;
    }

    if (child->display == VL_CSS_LAYOUT_DISPLAY_BLOCK) {
        span_flush_run(root, regions);
        return;
    }

    span_reset_node(child);

    if (child->display == VL_CSS_LAYOUT_DISPLAY_INLINE)
        span_collect_children(root, child, regions);
}

static void span_collect_children(vl_css_layout_node_t *root, vl_css_layout_node_t *parent, VL_DA(vl_css_layout_span_region_t) *regions) {
    if (!parent->children) return;

    for (int i = 0; i < VL_DA_LENGTH(parent->children); i++)
        span_collect_child(root, parent, parent->children[i], regions);
}

static void span_finish_nodes(vl_css_layout_node_t *node,
                              vl_css_layout_node_t *root) {
    if (!node || (node != root && node->display == VL_CSS_LAYOUT_DISPLAY_BLOCK))
        return;

    if (node->span_index == -2)
        node->span_index = node->span_index_last = -1;

    for (int i = 0; i < VL_DA_LENGTH(node->children); i++)
        span_finish_nodes(node->children[i], root);
}

static void recurse_spans(vl_css_layout_node_t *node) {
    span_reset_node(node);

    node->span_regions = VL_DA_INIT(VL_DA(vl_css_layout_span_region_t));
    if (!node->span_regions) return;

    VL_DA(vl_css_layout_span_region_t) regions = NULL;

    span_collect_children(node, node, &regions);
    span_flush_run(node, &regions);
    VL_DA_FREE(regions);

    span_finish_nodes(node, node);

    if (VL_DA_LENGTH(node->span_regions) == 1 &&
        node->span_regions[0] &&
        VL_DA_LENGTH(node->span_regions[0])) {
        VL_DA(vl_css_layout_span_region_t) run = node->span_regions[0];

        node->span_ref_region = run;
        node->span_ref_flatenned_text = run[0].owner
            ? run[0].owner->span_flattened_text : NULL;
    }
}

#endif // VELVET_CSS_BITS_COLLECT_SPANS_C