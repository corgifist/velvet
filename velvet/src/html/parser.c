#include "html/parser.h"
#include "html/document.h"
#include "html/lexer.h"
#include "support/da.h"
#include "support/error_pool.h"
#include "support/result.h"
#include "support/str.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <unicode/stringoptions.h>
#include <unicode/urename.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>


#define VL_TOKEN_COMPARE_EX(A, A_LENGTH, B) \
    ((A_LENGTH == sizeof(B) - 1) && (!vl_nstrcicmp(A, B, A_LENGTH)))

#define VL_TOKEN_COMPARE(TOKEN_PTR, B) \
    VL_TOKEN_COMPARE_EX((TOKEN_PTR)->text, (TOKEN_PTR)->text_length, (B))

#define VL_TOKEN_EMPTY(TOKEN) \
    VL_TOKEN_COMPARE(TOKEN, " ") || VL_TOKEN_COMPARE(TOKEN, "\t") || VL_TOKEN_COMPARE(TOKEN, "\n")

typedef struct {
    const char *k;
    const char *v;
} vl_html_parser_escape_t;

#define ESCAPE(K, V) ((vl_html_parser_escape_t) {.k = K, .v = V})

static const vl_html_parser_escape_t s_escapes[] = {
    ESCAPE("amp", "&"),
    ESCAPE("lt", "<"),
    ESCAPE("gt", ">"),
    ESCAPE("quot", "\""),
    ESCAPE("apos", "'"),
};

static vl_result_t tokenize(vl_html_parser_t *parser) {
    for (int i = 1; i < VL_HTML_PARSER_LOOKAHEAD; i++) {
        parser->lookahead[i - 1] = parser->lookahead[i];
    }
    vl_result_t result = vl_html_lexer_get(&parser->lexer, parser->lookahead + VL_HTML_PARSER_LOOKAHEAD - 1);
    if (result) {
        vl_error_pool_append(parser->ep, 0, 0, "vl_html_lexer_get failed");
    }
    return result;
}

static vl_result_t skip_spaces(vl_html_parser_t *parser) {
    vl_html_token_t *current = parser->lookahead;
    while (VL_TOKEN_EMPTY(current)) {
        // skip all meaningless tokens
        if (tokenize(parser)) return VL_ERROR;
    } 
    if (VL_TOKEN_COMPARE(current, "<") 
            && VL_TOKEN_COMPARE(current + 1, "!") 
            && VL_TOKEN_COMPARE(current + 2, "-") 
            && VL_TOKEN_COMPARE(current + 3, "-")) {
        // skip comment
        for (int i = 0; i < 4; i++) {
            if (tokenize(parser)) return VL_ERROR;
        }
        while (true) {
            if (VL_TOKEN_COMPARE(current, "-") && VL_TOKEN_COMPARE(current + 1, "-") && VL_TOKEN_COMPARE(current + 2, ">")) {
                break;
            }
            if (tokenize(parser)) return VL_ERROR;
        }
        for (int i = 0; i < 3; i++) {
            if (tokenize(parser)) return VL_ERROR;
        }
    }
    while (VL_TOKEN_EMPTY(current)) {
        // skip all meaningless tokens
        if (tokenize(parser)) return VL_ERROR;
    }
    return VL_SUCCESS;
}

vl_result_t vl_html_parser_init(vl_html_parser_t *parser, const char *input) {
    if (!parser || !input) return VL_ERROR;
    vl_html_lexer_t lexer = {0};
    if (parser->ep) {
        lexer.ep = parser->ep;
    }
    if (vl_html_lexer_init(&lexer, input)) return VL_ERROR;
    if (vl_html_parser_init_with_lexer(parser, lexer)) return VL_ERROR;
    return VL_SUCCESS;
}

vl_result_t vl_html_parser_init_with_lexer(vl_html_parser_t *parser, vl_html_lexer_t lexer) {
    if (!parser || !lexer.text) return VL_ERROR;
    parser->lexer = lexer;
    if (!parser->ep) {
        // parser can inherit lexer's error pool if parser->ep is NULL
        parser->ep = lexer.ep;
    }
    memset(parser->lookahead, 0, sizeof(parser->lookahead));
    for (int i = 0; i < VL_HTML_PARSER_LOOKAHEAD; i++) {
        if (vl_html_lexer_get(&parser->lexer, parser->lookahead + i)) {
            return VL_ERROR;
        }
    }
    return VL_SUCCESS;
}


static vl_result_t collect_escaped_string(vl_html_parser_t *parser, VL_DA(char)* text, bool include_quotes) {
    if (!parser || !text || !*text) return VL_ERROR;
    vl_html_token_t *current = parser->lookahead;
    bool double_quote = VL_TOKEN_COMPARE(current, "\"");
    if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip quote
    VL_DA(char) escape_accumulator = NULL;
    int limit = strlen(s_escapes->k);
    for (int i = 1; i < sizeof(s_escapes) / sizeof(*s_escapes); i++) {
        int len = strlen(s_escapes[i].k);
        if (len > limit) limit = len;
    }
    while (!((double_quote && VL_TOKEN_COMPARE(current, "\"") || (!double_quote && VL_TOKEN_COMPARE(current, "'"))))) {
        if (VL_TOKEN_COMPARE(current, "&") && !VL_TOKEN_COMPARE(current + 1, " ") && VL_TOKEN_COMPARE(current + 2, ";")) {
            if (tokenize(parser)) return VL_ERROR; // skip &
            if (!escape_accumulator) {
                escape_accumulator = VL_DA_INIT(char);
            }
            while (!VL_TOKEN_COMPARE(current, ";")) {
                for (int i = 0; i < current->text_length; i++) {
                    *VL_DA_PUSH(escape_accumulator, char) = current->text[i];
                }
                if (tokenize(parser)) goto escape_failure;
            }
            if (tokenize(parser)) goto escape_failure; // skip ;
            for (int j = 0; j < sizeof(s_escapes) / sizeof(*s_escapes); j++) {
                if (strlen(s_escapes[j].k) != VL_DA_LENGTH(escape_accumulator)) continue;
                if (memcmp(s_escapes[j].k, escape_accumulator, 
                        VL_DA_LENGTH(escape_accumulator)) == 0) {
                        for (int k = 0; k < strlen(s_escapes[j].v); k++) {
                            *VL_DA_PUSH(*text, char) = s_escapes[j].v[k];
                        }
                        goto escape_success;
                }
            }

            // escaping failed, adding the collected text back
            *VL_DA_PUSH(*text, char) = '&';
            for (int j = 0; j < VL_DA_LENGTH(escape_accumulator); j++) {
                *VL_DA_PUSH(*text, char) = escape_accumulator[j];
            }
            *VL_DA_PUSH(*text, char) = ';';

            escape_success:
            // resetting the escape_accumulator to avoid unnecessary allocations
            VL_DA_HEADER(escape_accumulator)->count = 0;
            continue;

            escape_failure:
            VL_DA_FREE(escape_accumulator);
            return VL_ERROR;
        }
        // funky workaround for case like: &quot ;
        if (VL_TOKEN_COMPARE(current, "&") && (current + 1)->type == VL_HTML_TOKEN_TYPE_WORD && !VL_TOKEN_COMPARE(current + 2, ";")) {
            if (tokenize(parser)) return VL_ERROR; // skip &
            for (int j = 0; j < sizeof(s_escapes) / sizeof(*s_escapes); j++) {
                if (strlen(s_escapes[j].k) != current->text_length) continue;
                if (memcmp(s_escapes[j].k, current->text, 
                        current->text_length) == 0) {
                        for (int k = 0; k < strlen(s_escapes[j].v); k++) {
                            *VL_DA_PUSH(*text, char) = s_escapes[j].v[k];
                        }
                        goto buggy_escape_success;
                }
            }
            buggy_escape_success:
            if (tokenize(parser)) return VL_ERROR; // skip entity names
            continue;
        }
        for (int i = 0; i < current->text_length; i++) {
            *VL_DA_PUSH(*text, char) = current->text[i];
        }
        if (tokenize(parser)) return VL_ERROR;
    }
    if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip closing quote
    end:
    if (escape_accumulator) VL_DA_FREE(escape_accumulator);
    return VL_SUCCESS;
}

static vl_result_t tokenize_node(vl_html_parser_t *parser, vl_html_node_t *node) {
    if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip <
    static const char *close_tag_begin = NULL;
    static const char *close_tag_end = NULL;
    vl_html_token_t *current = parser->lookahead;
    if (VL_TOKEN_COMPARE(current, "/")) {
        // parse closing node
        // e.g. </div>
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
        if (current->type != VL_HTML_TOKEN_TYPE_WORD) {
            vl_error_pool_append(parser->ep, current->line, current->inline_pos, "expected word while parsing closing node");
            return VL_ERROR;
        }
        close_tag_begin = current->text;
        close_tag_end = current->text + current->text_length;
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
        if (!VL_TOKEN_COMPARE(current, ">")) {
            vl_error_pool_append(parser->ep, current->line, current->inline_pos, "expected '>' while parsing closing node");
            return VL_ERROR;
        }
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
        return VL_HTML_PARSER_CLOSE_NODE;
    }
    if (VL_TOKEN_COMPARE(current, "!")) {
        // parse doctype node
        // e.g. <!doctype html>
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip !
        if (!VL_TOKEN_COMPARE(current, "doctype")) {
            vl_error_pool_append(parser->ep, current->line, current->inline_pos, "expected 'doctype' while parsing doctype");
            return VL_ERROR;
        }
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip doctype
        while (!VL_TOKEN_COMPARE(current, ">")) {
            if (current->type != VL_HTML_TOKEN_TYPE_WORD && !(VL_TOKEN_COMPARE(current, "\"") || VL_TOKEN_COMPARE(current, "'"))) {
                vl_error_pool_append(parser->ep, current->line, current->inline_pos, "expected word or string while parsing doctype");
                return VL_ERROR;
            }
            vl_html_attribute_t attribute = {0};
            attribute.value = VL_DA_INIT_WITH_CAPACITY(char, current->text_length + 1);
            memcpy(attribute.value, current->text, current->text_length);
            attribute.value[current->text_length] = '\0';
            if (tokenize(parser) || skip_spaces(parser)) {
                VL_DA_FREE(attribute.value);
                return VL_ERROR;
            }
            VL_DA_APPEND(node->attributes, attribute);
        }
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip >
        return VL_HTML_PARSER_DOCTYPE_NODE;
    }
    // printf("%i current: %.*s\n", current->type, current->text_length, current->text);
    if (current->type != VL_HTML_TOKEN_TYPE_WORD) {
        vl_error_pool_append(parser->ep, current->line, current->inline_pos, "expected word while parsing node");
        return VL_ERROR;
    }
    node->tag = VL_DA_INIT_WITH_CAPACITY(char, current->text_length + 1);
    VL_DA_HEADER(node->tag)->count = current->text_length + 1;
    memcpy(node->tag, current->text, current->text_length);
    node->tag[VL_DA_LENGTH(node->tag)] = '\0';
    if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip tag name and spaces
    // parsing node open
    while (!VL_TOKEN_COMPARE(current, ">")) {
        if (current->type == VL_HTML_TOKEN_TYPE_STOP) {
            return VL_SUCCESS;
        }
        if (VL_TOKEN_COMPARE(current, "/")) {
            // parse short node
            // e.g. <meta content="..." />
            if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip /
            if (!VL_TOKEN_COMPARE(current, ">")) {
                vl_error_pool_append(parser->ep, current->line, current->inline_pos, "expected '>' while parsing node");
                return VL_ERROR;
            }
            if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip >
            return VL_SUCCESS;
        }
        vl_html_attribute_t attribute = {0};
        if (current->type != VL_HTML_TOKEN_TYPE_WORD) {
            if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
            continue;
        }
        attribute.name = VL_DA_INIT_WITH_CAPACITY(char, current->text_length +1);
        VL_DA_HEADER(attribute.name)->count = current->text_length + 1;
        memcpy(attribute.name, current->text, current->text_length);
        attribute.name[current->text_length] = '\0';
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
        if (!VL_TOKEN_COMPARE(current, "=")) {
            // parsing empty attribute
            // e.g. <checkbox selected> or <script async>
            attribute.value = VL_DA_INIT_WITH_CAPACITY(char, 1);
            attribute.value[0] = '\0';
            goto append_attribute;
        }
        if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
        if (!VL_TOKEN_COMPARE(current, "\"") && !VL_TOKEN_COMPARE(current, "'")) {
            vl_error_pool_append(parser->ep, current->line, current->inline_pos, "expected string while parsing node attribute value");
            if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR;
            continue;
        }
        attribute.value = VL_DA_INIT(char);
        if (collect_escaped_string(parser, &attribute.value, false)) {
            vl_error_pool_append(parser->ep, current->line, current->inline_pos, "failed to parse node attribute value");
            return VL_ERROR;
        }

        append_attribute:
        VL_DA_APPEND(node->attributes, attribute);
    }

    if (tokenize(parser) || skip_spaces(parser)) return VL_ERROR; // skip >
    vl_html_node_t tmp_node = {0};
    while (true) {
        if (vl_html_node_init(&tmp_node)) return VL_ERROR;
        vl_result_t parse_result = vl_html_parser_get_ex(parser, &tmp_node);
        // printf("%i\n", parse_result);
        if (parse_result == VL_HTML_PARSER_STOP) {
            if (vl_html_node_deinit(&tmp_node)) return VL_ERROR;
            return VL_SUCCESS;
        }
        if (parse_result == VL_ERROR) {
            vl_html_node_deinit(&tmp_node);
            return VL_ERROR;
        }
        if (parse_result == VL_HTML_PARSER_CLOSE_NODE) {
            if (vl_html_node_deinit(&tmp_node)) return VL_ERROR;
            if (VL_DA_LENGTH(node->tag) - 1 != close_tag_end - close_tag_begin) {
                vl_error_pool_append(parser->ep, current->line, current->inline_pos, "begin/close node tag mismatch");
                return VL_ERROR;
            }
            if (memcmp(node->tag, close_tag_begin, VL_DA_LENGTH(node->tag) - 1) != 0) {
                vl_error_pool_append(parser->ep, current->line, current->inline_pos, "begin/close node tag mismath");
                return VL_ERROR;
            }
            return VL_SUCCESS;
        }
        VL_DA_APPEND(node->children, tmp_node);
    }

    return VL_SUCCESS;
}

static vl_result_t tokenize_text(vl_html_parser_t *parser, vl_html_node_t *node) {
    if (!parser || !node) return VL_ERROR;
    vl_html_token_t *current = parser->lookahead;
    node->text = VL_DA_INIT(char);
    while (true) {
        if (current->type == VL_HTML_TOKEN_TYPE_STOP) {
            // out of tokens
            goto success;
        }
        // parse html escapes (e.g. &apos;)
        if (VL_TOKEN_COMPARE(current, "&")) {
            VL_DA(char) entity_accumulator = VL_DA_INIT(char);
            if (tokenize(parser)) {  // skip &
                goto escape_fail;
            }
            while (!VL_TOKEN_COMPARE(current, ";")) {
                for (int i = 0; i < current->text_length; i++) {
                    *VL_DA_PUSH(entity_accumulator, char) = current->text[i];
                }
                if (tokenize(parser)) {
                    goto escape_fail;
                }
            }
            if (tokenize(parser)) { // skip ;
                goto escape_fail;
            }
            goto escape_success;

            escape_fail:
            VL_DA_FREE(entity_accumulator);
            VL_DA_FREE(node->text);
            vl_error_pool_append(parser->ep, current->line, current->inline_pos, "failed to process html string entity");
            return VL_ERROR;
            
            escape_success:
            for (int i = 0; i < sizeof(s_escapes) / sizeof(*s_escapes); i++) {
                if (strlen(s_escapes[i].k) != VL_DA_LENGTH(entity_accumulator)) continue;
                if (memcmp(entity_accumulator, 
                            s_escapes[i].k, 
                             VL_DA_LENGTH(entity_accumulator)) == 0) {
                    const char *v = s_escapes[i].v;
                    while (*v != '\0') {
                        *VL_DA_PUSH(node->text, char) = *v++;
                    }
                    goto escape_next;
                }
            }

            // push the collected text back
            *VL_DA_PUSH(node->text, char) = '&';
            for (int i = 0; i < VL_DA_LENGTH(entity_accumulator); i++) {
                *VL_DA_PUSH(node->text, char) = entity_accumulator[i];
            }
            *VL_DA_PUSH(node->text, char) = ';';

            escape_next:
            continue;
        }
        if (VL_TOKEN_COMPARE(current, "<") && (current + 1)->type == VL_HTML_TOKEN_TYPE_WORD) {
            // end of the text node, beginning of the tag node
            goto success;
        }
        if (VL_TOKEN_COMPARE(current, "<") && VL_TOKEN_COMPARE(current + 1, "/")
                && (current + 2)->type == VL_HTML_TOKEN_TYPE_WORD) {
            // end of the text node, beginning of the tag node
            goto success;
        }
        for (int i = 0; i < current->text_length; i++) {
            *VL_DA_PUSH(node->text, char) = current->text[i];
        }
        if (tokenize(parser)) return VL_ERROR;
        const char *original_text = current->text;
        if (skip_spaces(parser)) return VL_ERROR;
        if (original_text != current->text) {
            *VL_DA_PUSH(node->text, char) = ' ';
        }
    }

    success:
    *VL_DA_PUSH(node->text, char) = '\0';
    return VL_SUCCESS;
}

vl_result_t vl_html_parser_get_ex(vl_html_parser_t *parser, vl_html_node_t *node) {
    if (!parser || !node) return VL_ERROR;
    vl_html_token_t *current = parser->lookahead;
    if (current->type == VL_HTML_TOKEN_TYPE_STOP) {
        return VL_HTML_PARSER_STOP;
    }
    if (skip_spaces(parser)) return VL_ERROR;
    if (VL_TOKEN_COMPARE(current, "<") && (current + 1)->type == VL_HTML_TOKEN_TYPE_WORD 
            || VL_TOKEN_COMPARE(current, "<") && VL_TOKEN_COMPARE(current + 1, "/")
            || VL_TOKEN_COMPARE(current, "<") && VL_TOKEN_COMPARE(current + 1, "!")) {
        // parse html tag
        // <tag attr1="" attr2="">
        //     ...
        // </tag>
        return tokenize_node(parser, node);
    }

    return tokenize_text(parser, node);
}

vl_result_t vl_html_parser_get(vl_html_parser_t *parser, vl_html_node_t *node) {
    vl_result_t parse_result = vl_html_parser_get_ex(parser, node);
    vl_html_token_t *current = parser->lookahead;
    // no more input left
    if (parse_result == VL_HTML_PARSER_STOP) {
        return VL_SUCCESS;
    }
    // doctypes nodes are normal and not normal at the same time
    if (parse_result == VL_HTML_PARSER_DOCTYPE_NODE) {
        return VL_SUCCESS;
    }
    // we should not get a closing node here
    if (parse_result == VL_HTML_PARSER_CLOSE_NODE) {
        vl_error_pool_append(parser->ep, current->line, current->inline_pos, "unexpected close node");
        return VL_ERROR;
    }
    return parse_result;
}

vl_result_t vl_html_parser_deinit(vl_html_parser_t *parser) {
    if (!parser) return VL_ERROR;
    if (vl_html_lexer_deinit(&parser->lexer)) return VL_ERROR;
    return VL_SUCCESS;
}
