#include "velvet/css/lexer.h"
#include "support/error_pool.h"
#include "support/memory.h"
#include "support/result.h"
#include "support/str.h"

#include <unicode/urename.h>
#include <unicode/ustring.h>
#include <unicode/uchar.h>
#include <unicode/utf8.h>

vl_result_t vl_css_lexer_init_(vl_css_lexer_t *lexer, const char *text, vl_source_location_t loc, vl_error_pool_t *ep) {
    if (!lexer) return VL_ERROR;
    lexer->raw_length = strlen(text);
    lexer->text = vl_malloc(lexer->raw_length + 1, loc);
    memcpy(lexer->text, text, lexer->raw_length);
    lexer->text[lexer->raw_length] = '\0';
    lexer->c = 0;
    lexer->length = vl_u8strlen(text);
    lexer->pos = lexer->inline_pos = 0;
    lexer->raw_pos = 0;
    lexer->line = 1;
    lexer->c = -2;
    lexer->ep = ep;
    return VL_SUCCESS;
}

static vl_result_t lexer_advance(vl_css_lexer_t *lexer) {
    do { 
        if (lexer->raw_pos > lexer->raw_length) {
            vl_error_pool_append(lexer->ep, lexer->line, lexer->inline_pos, "css lexer EOF");
            return VL_ERROR;
        }
        U8_NEXT(lexer->text, lexer->raw_pos, lexer->raw_length, lexer->c);
        if (lexer->c >= 0) { 
            lexer->pos++;
            lexer->inline_pos++;
            break;
        }
    } while (lexer->c < 0 && lexer->raw_pos < lexer->raw_length);
    return VL_SUCCESS;
}


vl_result_t vl_css_lexer_get(vl_css_lexer_t *lexer, vl_css_token_t *token) {
    if (!lexer || !token) return VL_ERROR;
    if (!lexer->text) return VL_ERROR;

#define SET_TOKEN(TYPE, BEGIN, END) \
    do { \
        token->type = TYPE; \
        token->line = lexer->line; \
        token->inline_pos = lexer->inline_pos; \
        token->text = (BEGIN); \
        token->text_length = (END) - (BEGIN); \
    } while (0)

#define ADVANCE() \
    if (lexer_advance(lexer)) return VL_ERROR;

    // end of input
    if (lexer->c == -2) {
        ADVANCE();
    }
    const char *cursor = lexer->text + lexer->raw_pos;
    if (lexer->pos > lexer->length) {
        SET_TOKEN(VL_CSS_TOKEN_TYPE_STOP, NULL, NULL);
        return VL_SUCCESS;
    }
    if (lexer->c == '\n') {
        lexer->inline_pos = 0;
        lexer->line++;
        ADVANCE();
        return vl_css_lexer_get(lexer, token);
    }

    if (lexer->c == ' ' || lexer->c == '\t') {
        ADVANCE();
        return vl_css_lexer_get(lexer, token);
    }

    if (lexer->raw_pos + 2 >= lexer->raw_length && *cursor == '/' && *(cursor + 1) == '*') {
        // skip comments
        ADVANCE(); // skip /
        ADVANCE(); // skip *
        while (true) {
            if (lexer->c == '*') {
                ADVANCE();
                if (lexer->c == '/') {
                    ADVANCE();
                    return vl_css_lexer_get(lexer, token);
                }
            }
            ADVANCE();
        }
        return VL_ERROR;
    }

    if (u_isUAlphabetic(lexer->c) || (lexer->c == '-' && *cursor == '-')) {
        const char *word_begin = cursor - U8_LENGTH(lexer->c);
        const char *word_end = cursor;
        ADVANCE();
        while (u_isalnum(lexer->c) || lexer->c == '-') {
            word_end += U8_LENGTH(lexer->c);
            ADVANCE();
        }
        SET_TOKEN(VL_CSS_TOKEN_TYPE_ID, word_begin, word_end);
        return VL_SUCCESS;
    }

    if (lexer->c >= '0' && lexer->c <= '9') {
        const char *num_begin = cursor - U8_LENGTH(lexer->c);
        const char *num_end = cursor;
        ADVANCE();
        bool dot_found = false;
        while ((lexer->c >= '0' && lexer->c <= '9') || (!dot_found && lexer->c == '.')) {
            if (lexer->c == '.') dot_found = true;
            num_end += U8_LENGTH(lexer->c);
            ADVANCE();
        }
        if (dot_found && lexer->c == '.') {
            vl_error_pool_append(lexer->ep, lexer->line, lexer->inline_pos, "malformed number");
            return VL_ERROR;
        }
        SET_TOKEN(VL_CSS_TOKEN_TYPE_NUMBER, num_begin, num_end);
        return VL_SUCCESS;
    }

    if (lexer->c == '"' || lexer->c == '\'') {
        bool double_quote = (lexer->c == '"');
        ADVANCE(); // skip the quote
        const char *str_begin = cursor - U8_LENGTH(lexer->c);
        const char *str_end = cursor;
        while ((double_quote && lexer->c != '"') || (!double_quote && lexer->c != '\'')) {
            str_end += U8_LENGTH(lexer->c);
            ADVANCE();
        }
        str_end += U8_LENGTH(lexer->c);
        ADVANCE(); // skip the quote
        SET_TOKEN(VL_CSS_TOKEN_TYPE_STRING, str_begin, str_end);
        return VL_SUCCESS;
    }

    SET_TOKEN(VL_CSS_TOKEN_TYPE_SYMBOL, cursor - U8_LENGTH(lexer->c), cursor);
    ADVANCE();

#undef ADVANCE
#undef SET_TOKEN
    
    return VL_SUCCESS;
}

vl_result_t vl_css_lexer_deinit(vl_css_lexer_t *lexer) {
    if (!lexer) return VL_ERROR;
    if (lexer->text) vl_free(lexer->text);
    return VL_SUCCESS;
}