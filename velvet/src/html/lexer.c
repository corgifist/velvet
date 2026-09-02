#include "html/lexer.h"
#include "support/alphanum.h"
#include "support/error_pool.h"
#include "support/memory.h"
#include "support/result.h"
#include "support/str.h"
#include "support/alphanum.h"

vl_result_t vl_html_lexer_init(vl_html_lexer_t *lexer, const char *text) {
    if (!lexer) return VL_ERROR;
    lexer->raw_length = strlen(text);
    lexer->text = vl_malloc(lexer->raw_length + 1);
    memcpy(lexer->text, text, lexer->raw_length);
    lexer->text[lexer->raw_length] = '\0';
    lexer->c = 0;
    lexer->length = vl_u8strlen(text);
    lexer->pos = lexer->inline_pos = 0;
    lexer->raw_pos = 0;
    lexer->line = 1;
    lexer->c = -2;
    return VL_SUCCESS;
}

static vl_result_t lexer_advance(vl_html_lexer_t *lexer) {
    do { 
        U8_NEXT(lexer->text, lexer->raw_pos, lexer->raw_length, lexer->c);
        if (lexer->c != 0) { 
            lexer->pos++;
            lexer->inline_pos++;
            return VL_SUCCESS;
        }
    } while (lexer->c == 0 && lexer->raw_pos < lexer->raw_length);
    return VL_SUCCESS;
}

vl_result_t vl_html_lexer_get(vl_html_lexer_t *lexer, vl_html_token_t *token) {
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

    if (lexer->c == -2) {
        ADVANCE();
    }
    const char *cursor = lexer->text + lexer->raw_pos;
    // printf("cursor: %s %s\n", cursor, cursor - U8_LENGTH(lexer->c));
    if (lexer->pos > lexer->length || lexer->c == 0) {
        SET_TOKEN(VL_HTML_TOKEN_TYPE_STOP, NULL, NULL);
        return VL_SUCCESS;
    }
    
    if (lexer->c == '\n') {
        lexer->inline_pos = 0;
        lexer->line++;
    }

    if (vl_isalphabetical(lexer->c)) {
        const char *word_begin = cursor - U8_LENGTH(lexer->c);
        const char *word_end = cursor;
        ADVANCE();
        while (vl_isalphanum(lexer->c)) {
            word_end += U8_LENGTH(lexer->c);
            ADVANCE();
        }
        SET_TOKEN(VL_HTML_TOKEN_TYPE_WORD, word_begin, word_end);
        return VL_SUCCESS;
    }

    SET_TOKEN(VL_HTML_TOKEN_TYPE_SYMBOL, cursor - U8_LENGTH(lexer->c), cursor);
    ADVANCE();
    // printf("current char: %i %c %zu %zu\n", lexer->c, lexer->c, lexer->raw_pos, lexer->raw_length);

#undef ADVANCE
#undef SET_TOKEN
    
    return VL_SUCCESS;
}

vl_result_t vl_html_lexer_deinit(vl_html_lexer_t *lexer) {
    if (!lexer) return VL_ERROR;
    if (lexer->text) vl_free(lexer->text);
    return VL_SUCCESS;
}