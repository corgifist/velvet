#ifndef VELVET_CSS_LEXER_H
#define VELVET_CSS_LEXER_H

#include "velvet/support/memory.h"
#include "velvet/common.h"
#include "velvet/support/da.h"
#include "velvet/support/error_pool.h"
#include "velvet/support/variadic.h"

struct vl_css_lexer {
    char *text;
    size_t raw_length; // amount of bytes in text (text is NOT null-terminated)
    size_t length; // amount of utf-8 codepoints in text

    size_t raw_pos, pos, inline_pos, line;
    int32_t c;

    vl_error_pool_t *ep;
};

typedef struct vl_css_lexer vl_css_lexer_t;

enum vl_css_token_type {
    VL_CSS_TOKEN_TYPE_STOP = 0,
    VL_CSS_TOKEN_TYPE_ID,
    VL_CSS_TOKEN_TYPE_SYMBOL,
    VL_CSS_TOKEN_TYPE_NUMBER
};

typedef enum vl_css_token_type vl_css_token_type_t;

struct vl_css_token {
    vl_css_token_type_t type;
    size_t line, inline_pos;
    const char *text;
    int text_length;
};

typedef struct vl_css_token vl_css_token_t;

#define vl_css_lexer_init2(lexer, text) \
    vl_css_lexer_init_(lexer, text, VL_SOURCE_LOCATION_HERE, NULL)
#define vl_css_lexer_init3(lexer, text, loc) \
    vl_css_lexer_init_(lexer, text, loc, NULL)
#define vl_css_lexer_init4(lexer, text, loc, ep) \
    vl_css_lexer_init_(lexer, text, loc, ep)
#define vl_css_lexer_init(...) VL_VA_DISPATCH(vl_css_lexer_init, __VA_ARGS__)
VL_API vl_result_t vl_css_lexer_init_(vl_css_lexer_t *lexer, const char *text, vl_source_location_t loc, vl_error_pool_t *ep);
VL_API vl_result_t vl_css_lexer_get(vl_css_lexer_t *lexer, vl_css_token_t *token);
VL_API vl_result_t vl_css_lexer_deinit(vl_css_lexer_t *lexer);

#endif // VELVET_CSS_LEXER_H