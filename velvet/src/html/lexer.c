#include "html/lexer.h"
#include "support/memory.h"
#include "support/result.h"

#include <string.h>
#include <stdio.h>

#include <unicode/urename.h>
#include <unicode/ustring.h>
#include <unicode/uchar.h>
#include <unicode/utf8.h>

vl_result_t vl_html_lexer_init(vl_html_lexer_t *lexer, const char *text) {
    if (!lexer) return VL_ERROR;
    lexer->raw_length = strlen(text);
    if (lexer->raw_length == 0) return VL_ERROR;
    lexer->text = VL_MALLOC(lexer->raw_length);
    memcpy(lexer->text, text, lexer->raw_length);
    size_t count = 0, i = 0;
    lexer->c = 0;
    do {
        U8_NEXT(text, i, 0, lexer->c);
        if (lexer->c == 0) break;
        count++;
    } while (lexer->c >= 0);
    lexer->length = count;
    lexer->pos = lexer->inline_pos = 0;
    lexer->raw_pos = 0;
    lexer->line = 1;
    lexer->c = -2;
    return VL_SUCCESS;
}

static void lexer_advance(vl_html_lexer_t *lexer) {
    do { 
        U8_NEXT(lexer->text, lexer->raw_pos, lexer->raw_length, lexer->c);
        if (lexer->c >= 0) { 
            lexer->pos++;
            lexer->inline_pos++;
            break;
        }
    } while (lexer->c < 0 && lexer->raw_pos < lexer->raw_length);
}


vl_result_t vl_html_lexer_get(vl_html_lexer_t *lexer, vl_html_token_t *token) {
    if (!lexer || !token) return VL_ERROR;
    if (!lexer->text) return VL_ERROR;

#define SET_TOKEN(TYPE, BEGIN, END) \
    do { \
        token->type = TYPE; \
        token->line = lexer->line; \
        token->inline_pos = lexer->raw_pos - U8_LENGTH(lexer->c); \
        token->text = (BEGIN); \
        token->text_length = (END) - (BEGIN); \
    } while (0)

#define ADVANCE() \
    lexer_advance(lexer)

    // end of input
    if (lexer->c == -2) {
        ADVANCE();
    }
    const char *cursor = lexer->text + lexer->raw_pos;
    if (lexer->pos > lexer->length) {
        SET_TOKEN(VL_HTML_TOKEN_TYPE_STOP, NULL, NULL);
        return VL_SUCCESS;
    }
    if (lexer->c == '\n') {
        lexer->inline_pos = 0;
        lexer->line++;
    }

    if (u_isUAlphabetic(lexer->c)) {
        const char *word_begin = cursor - U8_LENGTH(lexer->c);
        const char *word_end = cursor;
        ADVANCE();
        while (u_isalnum(lexer->c)) {
            word_end += U8_LENGTH(lexer->c);
            ADVANCE();
        }
        SET_TOKEN(VL_HTML_TOKEN_TYPE_WORD, word_begin, word_end);
        return VL_SUCCESS;
    }

    if (lexer->c == '\"' || lexer->c == '\'') {
        bool double_quote = (lexer->c == '"');
        const char *string_begin = cursor - U8_LENGTH(lexer->c);
        const char *string_end = cursor;
        ADVANCE(); // skip ' or "
        while ((double_quote && lexer->c != '\"') || (!double_quote && lexer->c != '\'')) {
            if (lexer->c < 0) {
                return VL_ERROR;
            }
            string_end += U8_LENGTH(lexer->c);
            ADVANCE();
        }
        ADVANCE(); // skip ' or "
        string_end += 1; // include closing double quote
        SET_TOKEN(VL_HTML_TOKEN_TYPE_STRING, string_begin, string_end);
        return VL_SUCCESS;
    }

    SET_TOKEN(VL_HTML_TOKEN_TYPE_SYMBOL, cursor - U8_LENGTH(lexer->c), cursor);
    ADVANCE();

#undef ADVANCE
#undef SET_TOKEN
    
    return VL_SUCCESS;
}

vl_result_t vl_html_lexer_deinit(vl_html_lexer_t *lexer) {
    if (!lexer) return VL_ERROR;
    if (!lexer->text) return VL_ERROR;
    VL_FREE(lexer->text);
    return VL_SUCCESS;
}