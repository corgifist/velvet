#include "velvet/font/segmentation.h"
#include "support/global_error_pool.h"
#include "support/result.h"
#include "vendor/grapheme.h"
#include <stdint.h>

VL_API vl_result_t vl_font_segmentation_process_string(const char *string, size_t string_len, vl_font_segmentation_type_t break_type, vl_font_segmentation_breaks_t *breaks) {
    if (!string || !breaks || string_len <= 0) return VL_ERROR;
    if (!*breaks) {
        *breaks = VL_DA_INIT(vl_font_segmentation_break_t);
    }
    size_t ret = 0;
    for (size_t offset = 0; offset < string_len; offset += ret) {
        switch (break_type) {
        case VL_FONT_SEGMENTATION_WORD: {
            ret = grapheme_next_word_break_utf8(string + offset, SIZE_MAX);
            break;
        }
        case VL_FONT_SEGMENTATION_SENTENCE: {
            ret = grapheme_next_sentence_break_utf8(string + offset, SIZE_MAX);
            break;
        }
        case VL_FONT_SEGMENTATION_LINE: {
            ret = grapheme_next_line_break_utf8(string + offset, SIZE_MAX);
            break;
        }
        }
        *VL_DA_PUSH(*breaks, vl_font_segmentation_break_t) = (vl_font_segmentation_break_t) {
            .begin = offset,
            .end = offset + ret
        };
    }
    return VL_SUCCESS;
}