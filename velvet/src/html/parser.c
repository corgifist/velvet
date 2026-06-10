#include "html/parser.h"
#include "html/lexer.h"
#include "support/result.h"

#include <string.h>

#define VL_TOKEN_COMPARE_EX(A, A_LENGTH, B) \
    ((A_LENGTH == sizeof(B) - 1) && (memcmp(A, B, A_LENGTH)))

#define VL_TOKEN_COMPARE(TOKEN_PTR, B) \
    VL_TOKEN_COMPARE_EX((TOKEN_PTR)->text, (TOKEN_PTR)->text_length, (B))

vl_result_t vl_html_parser_init(vl_html_parser_t *parser, const char *input) {
    if (!parser || !input) return VL_ERROR;
    vl_html_lexer_t lexer = {0};
    if (vl_html_lexer_init(&lexer, input)) return VL_ERROR;
    if (vl_html_parser_init_with_lexer(parser, lexer)) return VL_ERROR;
    return VL_SUCCESS;
}

vl_result_t vl_html_parser_init_with_lexer(vl_html_parser_t *parser, vl_html_lexer_t lexer) {
    if (!parser || !lexer.text) return VL_ERROR;
    parser->lexer = lexer;
    parser->lookahead_empty = 1;
    memset(parser->lookahead, 0, sizeof(parser->lookahead));
    return VL_SUCCESS;
}

static vl_result_t tokenize(vl_html_parser_t *parser) {
    if (parser->lookahead_empty) {
        for (int i = 0; i < VL_HTML_PARSER_LOOKAHEAD; i++) {
            if (vl_html_lexer_get(&parser->lexer, parser->lookahead + i)) {
                return VL_ERROR;
            }
        }
        return VL_SUCCESS;
    }
    for (int i = 1; i < VL_HTML_PARSER_LOOKAHEAD; i++) {
        parser->lookahead[i - 1] = parser->lookahead[0];
    }
    if (vl_html_lexer_get(&parser->lexer, &parser->lookahead[VL_HTML_PARSER_LOOKAHEAD - 1])) {
        return VL_ERROR;
    }
    return VL_SUCCESS;
}

vl_result_t vl_html_parser_get(vl_html_parser_t *parser, vl_html_node_t *node) {
    if (!parser || tokenize(parser)) return VL_ERROR;
    vl_html_token_t *current = &parser->lookahead[0];
    if (current->type == VL_HTML_TOKEN_TYPE_STOP) {
        return VL_HTML_PARSER_STOP;
    }

    if (VL_TOKEN_COMPARE(current, " ") || VL_TOKEN_COMPARE(current, "\t") || VL_TOKEN_COMPARE(current, "\n")) {
        // skip useless tokens (newlines, tabs, spaces)
        return vl_html_parser_get(parser, node);
    }

    if (VL_TOKEN_COMPARE(current, "<") && !VL_TOKEN_COMPARE(&parser->lookahead[1], " ")) {
        // TODO: implement this shit already like broooo
        return VL_ERROR;
    }
}
