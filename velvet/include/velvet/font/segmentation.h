#ifndef VELVET_FONT_SEGMENTATION_H
#define VELVET_FONT_SEGMENTATION_H

#include "velvet/common.h"
#include "velvet/support/da.h"
#include "velvet/support/result.h"

enum vl_font_segmentation_type {
    VL_FONT_SEGMENTATION_WORD = 0,
    VL_FONT_SEGMENTATION_SENTENCE,
    VL_FONT_SEGMENTATION_LINE,
};

typedef enum vl_font_segmentation_type vl_font_segmentation_type_t;

struct vl_font_segmentation_break {
    size_t begin, end;
};

typedef struct vl_font_segmentation_break vl_font_segmentation_break_t;
typedef VL_DA(vl_font_segmentation_break_t) vl_font_segmentation_breaks_t;

VL_API vl_result_t vl_font_segmentation_process_string(const char *string, size_t string_len, vl_font_segmentation_type_t break_type, vl_font_segmentation_breaks_t *breaks);

#endif // VELVET_FONT_SEGMENTATION_H