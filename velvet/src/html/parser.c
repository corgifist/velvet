#include "html/parser.h"
#include "html/document.h"
#include "html/lexer.h"
#include "support/da.h"
#include "support/result.h"
#include "support/math.h"

#include <string.h>
#include <stdio.h>

#define VL_TOKEN_COMPARE_EX(A, A_LENGTH, B) \
    ((A_LENGTH == sizeof(B) - 1) && (!memcmp(A, B, A_LENGTH)))

#define VL_TOKEN_COMPARE(TOKEN_PTR, B) \
    VL_TOKEN_COMPARE_EX((TOKEN_PTR)->text, (TOKEN_PTR)->text_length, (B))

#define VL_HTML_PARSER_CLOSE_NODE -2
#define VL_HTML_PARSER_SKIP_NODE -3

static vl_result_t tokenize(vl_html_parser_t *parser) {
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
    memset(parser->lookahead, 0, sizeof(parser->lookahead));
    for (int i = 0; i < VL_HTML_PARSER_LOOKAHEAD; i++) {
        if (vl_html_lexer_get(&parser->lexer, parser->lookahead + i)) {
            return VL_ERROR;
        }
    }
    return VL_SUCCESS;
}


static vl_result_t tokenize_node(vl_html_parser_t *parser, vl_html_node_t *node) {
    if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip <
    static const char *close_tag_begin = NULL;
    static const char *close_tag_end = NULL;
    vl_html_token_t *current = parser->lookahead;
    if (VL_TOKEN_COMPARE(current, "/")) {
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
        if (current->type != VL_HTML_TOKEN_TYPE_WORD) {
            return VL_ERROR;
        }
        close_tag_begin = current->text;
        close_tag_end = current->text + current->text_length;
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
        if (!VL_TOKEN_COMPARE(current, ">")) return VL_ERROR;
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
        return VL_HTML_PARSER_CLOSE_NODE;
    }
    // printf("%i current: %.*s\n", current->type, current->text_length, current->text);
    if (current->type != VL_HTML_TOKEN_TYPE_WORD) return VL_ERROR;
    node->tag = VL_DA_INIT_WITH_CAPACITY(char, current->text_length + 1);
    VL_DA_HEADER_PTR(node->tag)->count = current->text_length + 1;
    memcpy(node->tag, current->text, current->text_length);
    node->tag[VL_DA_LENGTH(node->tag)] = '\0';
    if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip tag name and spaces
    // parsing node open
    while (!VL_TOKEN_COMPARE(current, ">")) {
        if (current->type == VL_HTML_TOKEN_TYPE_STOP) {
            return VL_SUCCESS;
        }
        vl_html_attribute_t attribute = {0};
        if (current->type != VL_HTML_TOKEN_TYPE_WORD) {
            if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
            continue;
        }
        attribute.name = VL_DA_INIT_WITH_CAPACITY(char, current->text_length +1);
        VL_DA_HEADER_PTR(attribute.name)->count = current->text_length + 1;
        memcpy(attribute.name, current->text, current->text_length);
        attribute.name[current->text_length] = '\0';
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
        if (!VL_TOKEN_COMPARE(current, "=")) {
            if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
            continue;
        }
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
        if (current->type != VL_HTML_TOKEN_TYPE_STRING) {
            if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
            continue;
        }
        attribute.value = VL_DA_INIT(char);
        for (int i = 0; i < current->text_length - 2; i++) {
            *VL_DA_PUSH(attribute.value, char) = current->text[i + 1];
        }
        attribute.value[current->text_length - 2] = '\0';
        VL_DA_APPEND(node->attributes, attribute);
    }
    if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip >
    vl_html_node_t tmp_node = {0};
    if (vl_html_node_init(&tmp_node)) return VL_ERROR;
    while (true) {
        vl_result_t parse_result = vl_html_parser_get(parser, &tmp_node);
        // printf("%i\n", parse_result);
        if (parse_result == VL_HTML_PARSER_STOP) {
            return VL_SUCCESS;
        }
        if (parse_result == VL_ERROR) return VL_ERROR;
        if (parse_result == VL_HTML_PARSER_CLOSE_NODE) {
            if (memcmp(node->tag, close_tag_begin, MIN(VL_DA_LENGTH(node->tag) - 1, close_tag_end - close_tag_begin)) != 0) {
                return VL_ERROR;
            }
            return VL_SUCCESS;
        }
        VL_DA_APPEND(node->children, tmp_node);
        if (vl_html_node_init(&tmp_node)) return VL_ERROR;
    }

    return VL_SUCCESS;
}

vl_result_t vl_html_parser_get(vl_html_parser_t *parser, vl_html_node_t *node) {
    if (!parser) return VL_ERROR;
    vl_html_token_t *current = parser->lookahead;
    if (current->type == VL_HTML_TOKEN_TYPE_STOP) {
        return VL_HTML_PARSER_STOP;
    }
    if (skip_spaces(parser)) return VL_ERROR;
    if (VL_TOKEN_COMPARE(current, "<") && !VL_TOKEN_COMPARE(current + 1, " ")) {
        // parse html tag
        // <tag attr1="" attr2="">
        //     ...
        // </tag>
        return tokenize_node(parser, node);
    }

    return VL_SUCCESS;
}

vl_result_t vl_html_parser_deinit(vl_html_parser_t *parser) {
    if (!parser) return VL_ERROR;
    if (vl_html_lexer_deinit(&parser->lexer)) return VL_ERROR;
    return VL_SUCCESS;
}
