#include "html/parser.h"
#include "html/lexer.h"
#include "support/da.h"
#include "support/result.h"

#include <string.h>
#include <stdio.h>

#define VL_TOKEN_COMPARE_EX(A, A_LENGTH, B) \
    ((A_LENGTH == sizeof(B) - 1) && (!memcmp(A, B, A_LENGTH)))

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
        parser->lookahead_empty = 0;
        return VL_SUCCESS;
    }
    for (int i = 1; i < VL_HTML_PARSER_LOOKAHEAD; i++) {
        parser->lookahead[i - 1] = parser->lookahead[i];
    }
    return vl_html_lexer_get(&parser->lexer, parser->lookahead + VL_HTML_PARSER_LOOKAHEAD - 1) == VL_ERROR;
}

static vl_result_t skip_spaces(vl_html_parser_t *parser) {
    vl_html_token_t *current = parser->lookahead;
    while (VL_TOKEN_COMPARE(current, " ") || VL_TOKEN_COMPARE(current, "\t") || VL_TOKEN_COMPARE(current, "\n")) {
        // skip all meaningless tokens
        if (tokenize(parser)) return VL_ERROR;
    } 
    return VL_SUCCESS;
}

static vl_result_t tokenize_node(vl_html_parser_t *parser, vl_html_node_t *node) {
    if (tokenize(parser)) return VL_ERROR; // skip <
    vl_html_token_t *current = parser->lookahead;
    printf("%i current: %.*s\n", current->type, current->text_length, current->text);
    node->tag = VL_DA_INIT_WITH_CAPACITY(char, current->text_length + 1);
    VL_DA_HEADER_PTR(node->tag)->count = current->text_length + 1;
    memcpy(node->tag, current->text, current->text_length);
    node->tag[VL_DA_LENGTH(node->tag)] = '\0';
    if (tokenize(parser)) return VL_ERROR; // skip tag name
    if (skip_spaces(parser)) return VL_ERROR;
    if (!VL_TOKEN_COMPARE(current, ">")) {
        return VL_ERROR;
    }
    return VL_SUCCESS;
}

vl_result_t vl_html_parser_get(vl_html_parser_t *parser, vl_html_node_t *node) {
    if (!parser || tokenize(parser)) return VL_ERROR;
    vl_html_token_t *current = parser->lookahead;
    if (current->type == VL_HTML_TOKEN_TYPE_STOP) {
        return VL_HTML_PARSER_STOP;
    }
    skip_spaces(parser);
    if (VL_TOKEN_COMPARE(current, "<")
            && (parser->lookahead + 1)->type == VL_HTML_TOKEN_TYPE_WORD) {
        // parse html tag
        // <tag attr1="" attr2="">
        //     ...
        // </tag>
        printf("enter tokenize_node\n");
        return tokenize_node(parser, node);
    }

    return VL_SUCCESS;
}

vl_result_t vl_html_parser_deinit(vl_html_parser_t *parser) {
    if (!parser) return VL_ERROR;
    if (vl_html_lexer_deinit(&parser->lexer)) return VL_ERROR;
    return VL_SUCCESS;
}
