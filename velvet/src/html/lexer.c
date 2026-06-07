#include "html/lexer.h"
#include "support/memory.h"
#include "velvet.h"

#include <string.h>

#include <unicode/umachine.h>
#include <unicode/ustring.h>

#include <stdio.h>

vl_result_t vl_html_lexer_init(vl_html_lexer_t *lexer, const char *text) {
    if (!lexer) return VL_ERROR;
    size_t text_len = strlen(text);
    if (text_len == 0) return VL_ERROR;
    printf("text_len: %zu\n", text_len);
    lexer->text = VL_MALLOC(text_len);
    memcpy(lexer->text, text, text_len);
    size_t count = 0, i = 0;
    UChar32 c;
    do {
        U8_NEXT(text, i, 0, c);
        if (c == 0) break;
        count++;
    } while (c >= 0);
    lexer->length = count;
    lexer->pos = lexer->inline_pos = 0;
    lexer->line = 1;
    return VL_SUCCESS;
}

vl_result_t vl_html_lexer_deinit(vl_html_lexer_t *lexer) {
    if (!lexer) return VL_ERROR;
    if (!lexer->text) return VL_ERROR;
    VL_FREE(lexer->text);
    return VL_SUCCESS;
}