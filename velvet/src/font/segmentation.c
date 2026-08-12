#include "velvet/font/segmentation.h"
#include "support/global_error_pool.h"
#include "support/result.h"

#include <unicode/ubrk.h>
#include <unicode/umachine.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include <unicode/ucsdet.h>

static UBreakIteratorType get_iter_type(vl_font_segmentation_type_t type) {
    switch (type) {
    case VL_FONT_SEGMENTATION_WORD: return UBRK_WORD;
    case VL_FONT_SEGMENTATION_SENTENCE: return UBRK_SENTENCE;
    case VL_FONT_SEGMENTATION_LINE: return UBRK_LINE;
    default: return UBRK_CHARACTER;
    }
}

vl_result_t vl_font_segmentation_process_string(const char *string, size_t string_len, vl_font_segmentation_type_t break_type, vl_font_segmentation_breaks_t *breaks) {
    if (!string || !breaks) return VL_ERROR;
    if (!*breaks) {
        *breaks = VL_DA_INIT(vl_font_segmentation_break_t);
    }
    UErrorCode err = U_ZERO_ERROR;
    int32_t dest_length;
    u_strFromUTF8(NULL, 0, &dest_length, string,
                                                -1, &err);
    if (err != U_BUFFER_OVERFLOW_ERROR && err != U_ZERO_ERROR) {
        vl_global_error_pool_append("utf-8 -> utf-16 pre-flight failed '%s' (error %i)", string, err);
        return VL_ERROR;
    }
    err = U_ZERO_ERROR;
    printf("%i %i %zu\n",err,  dest_length, string_len);
    UChar *u16_string = vl_malloc(sizeof(UChar) * dest_length);
    u_strFromUTF8(u16_string, dest_length, &dest_length, string, string_len, &err);
    if (U_FAILURE(err)) {
        vl_global_error_pool_append("utf-8 -> utf-16 failed '%s' (error %i)", string, err);
        return VL_ERROR;
    }

    UBreakIterator *iter = ubrk_open(get_iter_type(break_type), "", u16_string, dest_length, &err);
    if (U_FAILURE(err)) {
        vl_global_error_pool_append("failed to segment string '%s' (error %i)", string, err);
        return VL_ERROR;
    }
    int start = ubrk_first(iter);
    for (int end = ubrk_next(iter); end != UBRK_DONE; start = end, end = ubrk_next(iter)) {
        *VL_DA_PUSH(*breaks, vl_font_segmentation_break_t) = (vl_font_segmentation_break_t) {
            .begin = start, .end = end
        };
    }

    return VL_SUCCESS;
}