#include "velvet/font/segmentation.h"
#include "support/result.h"
#include "vendor/grapheme.h"
#include "vendor/utf8.h"
#include <stdint.h>

VL_API vl_result_t vl_font_segmentation_process_string(const char *string, size_t string_len, vl_font_segmentation_type_t break_type, vl_font_segmentation_breaks_t *breaks, vl_font_segmentation_index_type_t index_type) {
    if (!string || !breaks || string_len <= 0) return VL_ERROR;
    if (!*breaks) {
        *breaks = VL_DA_INIT(vl_font_segmentation_break_t);
    }
    size_t ret = 0;
    int codepoints_offset = 0;
    for (size_t offset = 0; offset < string_len; offset += ret) {
        switch (break_type) {
        case VL_FONT_SEGMENTATION_WORD: {
            ret = grapheme_next_word_break_utf8(string + offset, string_len - offset);
            break;
        }
        case VL_FONT_SEGMENTATION_SENTENCE: {
            ret = grapheme_next_sentence_break_utf8(string + offset, string_len - offset);
            break;
        }
        case VL_FONT_SEGMENTATION_LINE: {
            ret = grapheme_next_line_break_utf8(string + offset, string_len - offset);
            break;
        }
        }
        if (index_type == VL_FONT_SEGMENTATION_INDEX_TYPE_SOURCE) {
            *VL_DA_PUSH(*breaks, vl_font_segmentation_break_t) = (vl_font_segmentation_break_t) {
                .begin = offset,
                .end = offset + ret
            };
        } else if (index_type == VL_FONT_SEGMENTATION_INDEX_TYPE_CODEPOINT) {
            int begin = codepoints_offset;
            for (int i = 0; i < ret; codepoints_offset++) {
                i += utf8codepointcalcsize(string + offset + i);
            }
            *VL_DA_PUSH(*breaks, vl_font_segmentation_break_t) = (vl_font_segmentation_break_t) {
                .begin = begin,
                .end = codepoints_offset
            };
        }
    }
    return VL_SUCCESS;
}