#ifndef VELVET_CSS_PARSER_H
#define VELVET_CSS_PARSER_H

#include "velvet/css/style.h"
#include "velvet/support/error_pool.h"
#include "velvet/support/memory.h"
#include "velvet/css/lexer.h"
#include "velvet/support/variadic.h"

#define VL_CSS_PARSER_LOOKAHEAD 3

struct vl_css_parser {
    vl_css_lexer_t lexer;

    vl_css_token_t lookahead[VL_CSS_PARSER_LOOKAHEAD];
    vl_error_pool_t *ep;
    int max_priority;
};

typedef struct vl_css_parser vl_css_parser_t;

#define vl_css_parser_init2(parser, text) \
    vl_css_parser_init_(parser, text, VL_SOURCE_LOCATION_HERE, NULL)
#define vl_css_parser_init3(parser, text, loc) \
    vl_css_parser_init_(parser, text, loc, NULL)
#define vl_css_parser_init4(parser, text, loc, ep) \
    vl_css_parser_init_(parser, text, loc, ep)
#define vl_css_parser_init(...) \
    VL_VA_DISPATCH(vl_css_parser_init, __VA_ARGS__)
VL_API vl_result_t vl_css_parser_init_(vl_css_parser_t *parser, const char *text, vl_source_location_t loc, vl_error_pool_t *ep);
VL_API vl_result_t vl_css_parser_get(vl_css_parser_t *parser, vl_css_class_t *class);
VL_API vl_result_t vl_css_parser_deinit(vl_css_parser_t *parser);

#endif // VELVET_CSS_PARSER_H