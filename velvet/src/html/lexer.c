#include "html/lexer.h"
#include "support/memory.h"

#include <string.h>
#include <stdio.h>

#include <unicode/ustring.h>

vl_result_t vl_html_lexer_init(vl_html_lexer_t *lexer, const char *text) {
    if (!lexer) return VL_ERROR;
    lexer->raw_length = strlen(text);
    if (lexer->raw_length == 0) return VL_ERROR;
    lexer->text = VL_MALLOC(lexer->raw_length);
    memcpy(lexer->text, text, lexer->raw_length);
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

vl_result_t vl_html_lexer_get(vl_html_lexer_t *lexer, vl_html_token_t *token) {
    if (!lexer || !token) return VL_ERROR;
    if (!lexer->text) return VL_ERROR;

    UChar32 c;
    const char *cursor = lexer->text + lexer->raw_pos;
    size_t cursor_inline_pos = lexer->inline_pos;

#define SET_TOKEN(TYPE, LENGTH) \
    do { \
        token->type = TYPE; \
        token->line = lexer->line; \
        token->inline_pos = cursor_inline_pos; \
        token->text = cursor; \
        token->text_length = LENGTH; \
    } while (0)

#define ADVANCE() \
    do { \
        U8_NEXT(lexer->text, lexer->raw_pos, lexer->raw_length, c); \
        if (c >= 0) { \
            lexer->pos++; \
            lexer->inline_pos++; \
            break; \
        } \
    } while (c < 0)

    // end of input
    if (lexer->pos >= lexer->length) {
        SET_TOKEN(VL_HTML_TOKEN_TYPE_STOP, 0);
        return VL_SUCCESS;
    }
    if (c == '\n') {
        lexer->inline_pos = 0;
        lexer->line++;
    }

    ADVANCE();

    SET_TOKEN(VL_HTML_TOKEN_TYPE_SYMBOL, U8_LENGTH(c));
    
    return VL_SUCCESS;
}

vl_result_t vl_html_lexer_deinit(vl_html_lexer_t *lexer) {
    if (!lexer) return VL_ERROR;
    if (!lexer->text) return VL_ERROR;
    VL_FREE(lexer->text);
    return VL_SUCCESS;
}