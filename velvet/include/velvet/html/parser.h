#ifndef VELVET_HTML_PARSER_H
#define VELVET_HTML_PARSER_H

#include "velvet/html/lexer.h"
#include "velvet/html/document.h"

#define VL_HTML_PARSER_STOP -1

#define VL_HTML_PARSER_LOOKAHEAD 3

struct vl_html_parser {
    vl_html_lexer_t lexer;

    vl_html_token_t lookahead[VL_HTML_PARSER_LOOKAHEAD];
};

typedef struct vl_html_parser vl_html_parser_t;

vl_result_t vl_html_parser_init(vl_html_parser_t *parser, const char *input);
vl_result_t vl_html_parser_init_with_lexer(vl_html_parser_t *parser, vl_html_lexer_t lexer);

vl_result_t vl_html_parser_get(vl_html_parser_t *parser, vl_html_node_t *node);

vl_result_t vl_html_parser_deinit(vl_html_parser_t *parser);

#endif // VELVET_HTML_PARSER_H