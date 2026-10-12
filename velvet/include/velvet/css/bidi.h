#ifndef VELVET_CSS_BIDI_H
#define VELVET_CSS_BIDI_H

#include "velvet/css/layout.h"

typedef struct vl_css_bidi_context vl_css_bidi_context_t;

VL_API vl_css_bidi_context_t *vl_css_bidi_context_create(vl_css_layout_node_t *node);
VL_API void vl_css_bidi_context_destroy(vl_css_bidi_context_t *context);
VL_API void vl_css_bidi_reset_node(vl_css_layout_node_t *node);
VL_API void vl_css_bidi_accumulate_child(vl_css_layout_node_t *parent,
                                         const vl_css_layout_node_t *child);
VL_API void vl_css_bidi_finalize_node(vl_css_layout_node_t *node,
                                      const vl_css_bidi_context_t *context);
VL_API void vl_css_bidi_reorder_inline_line(vl_css_layout_node_t *node,
                                            vl_css_bidi_context_t *context,
                                            vl_css_layout_node_t **elements,
                                            int element_count);

#endif
