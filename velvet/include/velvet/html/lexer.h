#ifndef VELVET_HTML_LEXER_H
#define VELVET_HTML_LEXER_H

#include <stdlib.h>

#include "velvet/support/result.h"

struct vl_html_lexer {
    char *text;
    size_t length;
    size_t pos, inline_pos, line;
};

typedef struct vl_html_lexer vl_html_lexer_t;

enum vl_html_token_type {
    VL_HTML_TOKEN_TYPE_SYMBOL
};

typedef enum vl_html_token_type vl_html_token_type_t;

struct vl_html_token {
    vl_html_token_type_t type;
    size_t line, inline_pos;
    const char *text;
    size_t text_length;
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
 * deinitialize a given vl_html_lexer instance
 *
 * @param lexer a pointer to the vl_html_lexer
 * @return vl_result code
 */
vl_result_t vl_html_lexer_deinit(vl_html_lexer_t *lexer);

#endif // VELVET_HTML_LEXER_H