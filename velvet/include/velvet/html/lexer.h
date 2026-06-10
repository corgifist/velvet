#ifndef VELVET_HTML_LEXER_H
#define VELVET_HTML_LEXER_H

#include "velvet/support/result.h"

#include <stdlib.h>

#include <unicode/unistr.h>

struct vl_html_lexer {
    char *text;
    size_t raw_length; // amount of bytes in text (text is NOT null-terminated)
    size_t length; // amount of utf-8 codepoints in text
    size_t raw_pos, pos, inline_pos, line;
};

typedef struct vl_html_lexer vl_html_lexer_t;

enum vl_html_token_type {
    VL_HTML_TOKEN_TYPE_STOP = 0,
    VL_HTML_TOKEN_TYPE_SYMBOL
};

typedef enum vl_html_token_type vl_html_token_type_t;

struct vl_html_token {
    vl_html_token_type_t type;
    size_t line, inline_pos;
    const char *text;
    int text_length;
};

typedef struct vl_html_token vl_html_token_t;

/**
 * initialize a given vl_html_lexer instance
 *
 * @param lexer a pointer to the vl_html_lexer instance
 * @param text a utf-8 null-terminated string to be duplicated into lexer
 * @see vl_html_lexer
 * @return vl_result code
 */

vl_result_t vl_html_lexer_init(vl_html_lexer_t *lexer, const char *text);

/**
 * perform lexical analysis and return the next token
 *
 * @param lexer a pointer to the vl_html_lexer instance
 * @param token a pointer to the vl_html_token instance (token will be stored here)
 * @return vl_result code
 */
vl_result_t vl_html_lexer_get(vl_html_lexer_t *lexer, vl_html_token_t *token);

/**
 * deinitialize a given vl_html_lexer instance
 * deinitializing lexer invalidates all tokens produced by this lexer
 *
 * @param lexer a pointer to the vl_html_lexer
 * @return vl_result code
 */
vl_result_t vl_html_lexer_deinit(vl_html_lexer_t *lexer);

#endif // VELVET_HTML_LEXER_H