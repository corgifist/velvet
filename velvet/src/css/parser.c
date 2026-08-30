#include "velvet/css/parser.h"
#include "css/lexer.h"
#include "css/style.h"
#include "support/da.h"
#include "support/memory.h"
#include "support/result.h"
#include "support/str.h"
#include <iso646.h>
#include <stdlib.h>

#define VL_TOKEN_COMPARE_EX(A, A_LENGTH, B) \
    ((A_LENGTH == sizeof(B) - 1) && (!vl_nstrcicmp(A, B, A_LENGTH)))

#define VL_TOKEN_COMPARE(TOKEN_PTR, B) \
    VL_TOKEN_COMPARE_EX((TOKEN_PTR)->text, (TOKEN_PTR)->text_length, (B))
#define VL_TOKEN_CONSUME(PARSER, CHAR, FAIL) \
    do { \
        if (!VL_TOKEN_COMPARE((PARSER)->lookahead, CHAR)) { \
            vl_error_pool_append((PARSER)->ep, (PARSER)->lookahead->line, (PARSER)->lookahead->inline_pos, \
                "expected '%s' but got '%.*s' while parsing css stylesheet", \
                CHAR, (PARSER)->lookahead->text_length, (PARSER)->lookahead->text); \
            FAIL; \
        } \
        if (tokenize(PARSER)) { \
            FAIL; \
        } \
    } while (0)

static vl_result_t tokenize(vl_css_parser_t *parser) {
    for (int i = 1; i < VL_CSS_PARSER_LOOKAHEAD; i++) {
        parser->lookahead[i - 1] = parser->lookahead[i];
    }
    vl_result_t result = vl_css_lexer_get(&parser->lexer, parser->lookahead + VL_CSS_PARSER_LOOKAHEAD - 1);
    if (result) {
        vl_error_pool_append(parser->ep, 0, 0, "css parser tokenize() failed");
    }
    return result;
}

vl_result_t vl_css_parser_init_(vl_css_parser_t *parser, const char *text, vl_source_location_t loc, vl_error_pool_t *ep) {
    if (!parser) return VL_ERROR;
    if (vl_css_lexer_init(&parser->lexer, text, loc, ep)) return VL_ERROR;
    memset(parser->lookahead, 0, sizeof(parser->lookahead));
    for (int i = 0; i < VL_CSS_PARSER_LOOKAHEAD; i++) {
        if (vl_css_lexer_get(&parser->lexer, parser->lookahead + i)) {
            return VL_ERROR;
        }
    }
    parser->max_priority = 1;
    return VL_SUCCESS;
}

static vl_css_size_metric_type_t map_str_to_metric_type(const char *str) {
    static const struct {
        const char *unit;
        vl_css_size_metric_type_t type;
    } s_metric_unit_map[] = {
        "px", VL_CSS_SIZE_METRIC_PIXELS,
        "%", VL_CSS_SIZE_METRIC_PERCENTAGE,
        "em", VL_CSS_SIZE_METRIC_EM,
        "rem", VL_CSS_SIZE_METRIC_REM
    };

    for (int i = 0; i < VL_ARR_LEN(s_metric_unit_map); i++) {
        if (memcmp(str, s_metric_unit_map[i].unit, strlen(s_metric_unit_map[i].unit)) == 0) {
            return s_metric_unit_map[i].type;
        }
    }

    return VL_CSS_SIZE_METRIC_NONE;
}

static vl_css_value_t parse_single_metric(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    vl_css_token_t *current = parser->lookahead;
    float value = strtod(current->text, NULL);
    if (tokenize(parser)) return VL_CSS_VALUE_NONE();
    if (current->type != VL_CSS_TOKEN_TYPE_ID) {
        return VL_CSS_VALUE_METRIC1(
            VL_CSS_SIZE_PIXELS(value)
        );
    }
    vl_css_size_metric_type_t metric_type = map_str_to_metric_type(current->text);
    if (tokenize(parser) || metric_type == VL_CSS_SIZE_METRIC_NONE) return VL_CSS_VALUE_NONE();
    if (metric_type == VL_CSS_SIZE_METRIC_PERCENTAGE) metric_type /= 100.0f;
    return VL_CSS_VALUE_METRIC1(
        VL_CSS_SIZE_METRIC(metric_type, value)
    );
}

static VL_DA_STRING parse_id_or_string(vl_css_parser_t *parser) {
    vl_css_token_t *current = parser->lookahead;
    if (current->type == VL_CSS_TOKEN_TYPE_ID || current->type == VL_CSS_TOKEN_TYPE_STRING) {
        const char *begin = current->text;
        int len = current->text_length;
        if (current->type == VL_CSS_TOKEN_TYPE_STRING) {
            begin++;
            len -= 2;
        }
        VL_DA_STRING result = VL_DA_INIT_FROM_STRING_WITH_SIZE(begin, len);
        tokenize(parser);
        return result;
    }
    return NULL;
}

#define VL_CSS_RGBA_CONST(CONST, R, G, B, A) \
    VL_CSS_VALUE(VL_CSS_VALUE_COLOR_RGBA, CONST, {.rgba = VL_CSS_COLOR_RGBA(R, G, B, A)})

static const struct {
    const char *name;
    vl_css_value_t value;
} s_css_constants[] = {
    {"red", VL_CSS_VALUE_RGBA(255, 0, 0, 1)},
    {"green", VL_CSS_VALUE_RGBA(0, 128, 0, 1)},
    {"blue", VL_CSS_VALUE_RGBA(0, 0, 255, 1)},
    {"yellow", VL_CSS_VALUE_RGBA(255, 255, 0, 1)},
    {"aqua", VL_CSS_VALUE_RGBA(0, 255, 255, 1)},
    {"aquamarine", VL_CSS_VALUE_RGBA(127, 255, 212, 1)},
    {"fuchsia", VL_CSS_VALUE_RGBA(255, 0, 255, 1)},
    {"white", VL_CSS_VALUE_RGBA(255, 255, 255, 1)},
    {"brown", VL_CSS_VALUE_RGBA(165, 42, 42, 1)},
    {"whitesmoke", VL_CSS_VALUE_RGBA(245, 245, 245, 1)},
    {"turquoise", VL_CSS_VALUE_RGBA(64, 224, 208, 1)},
    {"wheat", VL_CSS_VALUE_RGBA(245, 222, 179, 1)},
    {"salmon", VL_CSS_VALUE_RGBA(250, 128, 114, 1)},
    {"snow", VL_CSS_VALUE_RGBA(255, 250, 250, 1)},
    {"powderblue", VL_CSS_VALUE_RGBA(176, 224, 230, 1)},
    {"lavender", VL_CSS_VALUE_RGBA(230, 230, 250, 1)},
    {"highlight-red", VL_CSS_VALUE_RGBA(128, 0, 0, 0.1)},
    {"highlight-green", VL_CSS_VALUE_RGBA(0, 128, 0, 0.1)},
    {"highlight-blue", VL_CSS_VALUE_RGBA(0, 0, 128, 0.1)}
};

static vl_css_value_t parse_generic_color(vl_css_parser_t *parser, vl_css_rule_t *rule, int max_components) {
    if (tokenize(parser) || tokenize(parser)) goto fail; // skip 'rgba' / 'rgba' and '('
    vl_css_token_t *current = parser->lookahead;
    float components[4] = {0.0, 0.0, 0.0, 1.0};
    int component = 0;
    while (!VL_TOKEN_COMPARE(current, ")")) {
        if (component >= 4) goto fail;
        if (current->type != VL_CSS_TOKEN_TYPE_NUMBER) goto fail;
        float value = strtod(current->text, NULL);
        components[component++] = value;
        if (tokenize(parser)) goto fail; // skip the number
        if (VL_TOKEN_COMPARE(current, ",") || VL_TOKEN_COMPARE(current, "/")) {
            if (tokenize(parser)) goto fail; // skip possible delimiter
        }
    }
    if (tokenize(parser)) goto fail; // skip ')'
    switch (max_components) {
        case 4: return VL_CSS_VALUE_RGBA(components[0], components[1], components[2], components[3]);
    }
    fail:
    return VL_CSS_VALUE_NONE();
}

static const char *s_const_literals[] = {
    "inherit",
    "initial",
    "unset",
    "revert",
    "canvastext",
    "block",
    "none"
};

static const char *try_parse_const_literal(vl_css_parser_t *parser, int limit) {
    vl_css_token_t *current = parser->lookahead;
    if (current->type == VL_CSS_TOKEN_TYPE_ID) {
        int count = limit;
        if (count <= 0) {
            count = VL_ARR_LEN(s_const_literals);
        }
        for (int i = 0; i < count; i++) {
            int const_len = strlen(s_const_literals[i]);
            if (const_len == current->text_length && vl_nstrcicmp(s_const_literals[i], current->text, const_len) == 0) {
                tokenize(parser);
                return s_const_literals[i];
            }
        }
    }
    return NULL;
}

static vl_css_value_t parse_primary_value(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    vl_css_token_t *current = parser->lookahead;
    if (current->type == VL_CSS_TOKEN_TYPE_ID) {
        for (int i = 0; i < VL_ARR_LEN(s_css_constants); i++) {
            if (current->text_length == strlen(s_css_constants[i].name) && 
                    vl_nstrcicmp(current->text, s_css_constants[i].name, current->text_length) == 0) {
                if (tokenize(parser)) return VL_CSS_VALUE_NONE();
                return s_css_constants[i].value;
            }
        }
    }
    if (current->type == VL_CSS_TOKEN_TYPE_NUMBER && (current + 1)->type == VL_CSS_TOKEN_TYPE_ID) {
        // single metric: 10px / 5em / 25%
        return parse_single_metric(parser, rule);
    }

    if (VL_TOKEN_COMPARE(current, "rgba") && VL_TOKEN_COMPARE(current + 1, "(")) {
        return parse_generic_color(parser, rule, 4);
    }

    const char *try_const_literal = try_parse_const_literal(parser, -1);
    if (try_const_literal) return VL_CSS_VALUE_CONST_LITERAL(try_const_literal);

    fail:
    return VL_CSS_VALUE_NONE();
}

static vl_css_value_t parse_shorthand_metric4(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    vl_css_size_metric_t max_metric[4];
    int metric_count = 0;

    vl_css_token_t *current = parser->lookahead;
    for (int i = 0; i < VL_ARR_LEN(max_metric); i++) {
        if (current->type != VL_CSS_TOKEN_TYPE_NUMBER)
            break;
        vl_css_value_t single_metric = parse_single_metric(parser, rule);
        if (single_metric.type != VL_CSS_VALUE_SIZE_METRIC1) {
            break;
        }
        vl_css_size_metric_t metric_value = single_metric.as.metric1;
        max_metric[metric_count++] = metric_value;
    }

    switch (metric_count) {
    case 1: return VL_CSS_VALUE_METRIC1(max_metric[0]);
    case 2: return VL_CSS_VALUE_METRIC2(max_metric[0], max_metric[1]);
    case 3: return VL_CSS_VALUE_METRIC3(max_metric[0], max_metric[1], max_metric[2]);
    case 4: return VL_CSS_VALUE_METRIC4(max_metric[0], max_metric[1], max_metric[2], max_metric[3]);
    }

    return VL_CSS_VALUE_NONE();
}

static vl_css_value_t parse_font_list(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    const char *global_const_literal = try_parse_const_literal(parser, 4);
    if (global_const_literal) return VL_CSS_VALUE_CONST_LITERAL(global_const_literal);
    vl_css_token_t *current = parser->lookahead;
    if (current->type == VL_CSS_TOKEN_TYPE_ID && VL_TOKEN_COMPARE(current + 1, ";")) {
        const char *literal = VL_DA_INIT_FROM_STRING_WITH_SIZE(current->text, current->text_length);
        tokenize(parser);
        return (vl_css_value_t) {
            .type = VL_CSS_VALUE_DYNAMIC_LITERAL,
            .as = {.const_literal = literal}
        };
    }
    vl_css_value_t result = {.type = VL_CSS_VALUE_FONT_LIST, .as = {0}};
    vl_css_font_list_t *font_list = &result.as.font_list;
    font_list->fonts = VL_DA_INIT(VL_DA_STRING);
    while (!VL_TOKEN_COMPARE(current, ";")) {
        VL_DA_STRING id = parse_id_or_string(parser);
        if (id) *VL_DA_PUSH(font_list->fonts, VL_DA_STRING) = id;
        else if (tokenize(parser)) break;
        goto next;
        next:
        if (VL_TOKEN_COMPARE(current, ",")) {
            if (tokenize(parser)) break;
        }
    }
    return result;
}

typedef vl_css_value_t (*vl_parse_value_with_context)(vl_css_parser_t *parser, vl_css_rule_t *rule);
static const struct {
    const char *property;
    vl_parse_value_with_context parse_with_context;
} s_context_table[] = {
    {"padding", parse_shorthand_metric4},
    {"margin", parse_shorthand_metric4},
    {"font-family", parse_font_list}
};

static vl_css_value_t dispatch_parse_value(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    for (int i = 0; i < VL_ARR_LEN(s_context_table); i++) {
        if (strcmp(s_context_table[i].property, rule->property) == 0) {
            return s_context_table[i].parse_with_context(parser, rule);
        }
    }

    return parse_primary_value(parser, rule);
}

static vl_result_t parse_rule(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    vl_css_token_t *current = parser->lookahead;
    if (current->type != VL_CSS_TOKEN_TYPE_ID) return VL_ERROR;
    rule->property = VL_DA_INIT_FROM_STRING_WITH_SIZE(current->text, current->text_length);
    if (tokenize(parser)) goto fail;
    VL_TOKEN_CONSUME(parser, ":", goto fail);
    rule->value = dispatch_parse_value(parser, rule);
    if (rule->value.type == VL_CSS_VALUE_NONE) goto fail;
    if (VL_TOKEN_COMPARE(current, "!") && VL_TOKEN_COMPARE(current + 1, "important")) {
        if (tokenize(parser) || tokenize(parser)) goto fail;
        rule->important = true;
    }
    return VL_SUCCESS;
    fail:
    vl_css_rule_deinit(rule);
    return VL_ERROR;
}

static vl_result_t parse_class_id(vl_css_parser_t *parser, vl_css_class_id_t *id) {
    vl_css_token_t *current = parser->lookahead;

    if (VL_TOKEN_COMPARE(current, "*")) {
        id->type = VL_CSS_CLASS_ID_ALL;
        id->name = NULL;
        if (tokenize(parser)) goto fail;
        return VL_SUCCESS;
    }
    if (current->type == VL_CSS_TOKEN_TYPE_ID) {
        id->type = VL_CSS_CLASS_ID_ELEMENT;
        id->name = VL_DA_INIT_FROM_STRING_WITH_SIZE(current->text, current->text_length);
        if (tokenize(parser)) goto fail;
        return VL_SUCCESS;
    }
    if (VL_TOKEN_COMPARE(current, ".") && (current + 1)->type == VL_CSS_TOKEN_TYPE_ID) {
        if (tokenize(parser)) goto fail; // skip '.'
        id->type = VL_CSS_CLASS_ID_CLASS;
        id->name = VL_DA_INIT_FROM_STRING_WITH_SIZE(current->text, current->text_length);
        if (tokenize(parser)) goto fail; // skip id
        return VL_SUCCESS;
    }

    fail:
    return VL_ERROR;
}

static vl_result_t parse_class_selector(vl_css_parser_t *parser, vl_css_class_selector_t *selector) {
    if (!selector->id_chain) {
        selector->id_chain = VL_DA_INIT(vl_css_class_id_t);
    }
    while (true) {
        vl_css_class_id_t id = {0};
        if (VL_TOKEN_COMPARE(parser->lookahead, ",") || VL_TOKEN_COMPARE(parser->lookahead, "{")) break; // moving onto the next selector
        if (parse_class_id(parser, &id)) goto fail;
        VL_DA_APPEND(selector->id_chain, id);
    }
    return VL_SUCCESS;
    fail:
    vl_css_class_selector_deinit(selector);
    return VL_ERROR;
}

static vl_result_t parse_class_selectors(vl_css_parser_t *parser, vl_css_class_t *class) {
    vl_css_token_t *current = parser->lookahead;
    if (!class->selectors) {
        class->selectors = VL_DA_INIT(vl_css_class_selector_t);
    }
    while (!VL_TOKEN_COMPARE(current, "{")) {
        vl_css_class_selector_t selector = {0};
        if (parse_class_selector(parser, &selector)) goto fail;
        VL_DA_APPEND(class->selectors, selector); 
        if (VL_TOKEN_COMPARE(current, ",") && tokenize(parser)) goto fail;
    }
    return VL_SUCCESS;
    fail:
    vl_css_class_deinit(class);
    return VL_ERROR;
}

vl_result_t vl_css_parser_get(vl_css_parser_t *parser, vl_css_class_t *class) {
    if (!parser) return VL_ERROR;
    vl_css_token_t *current = parser->lookahead;
    if (current->type == VL_CSS_TOKEN_TYPE_STOP) {
        return VL_STOP;
    }
    if (parse_class_selectors(parser, class)) goto fail;
    VL_TOKEN_CONSUME(parser, "{", goto fail);
    class->rules = VL_DA_INIT(vl_css_rule_t);
    int class_priority = parser->max_priority++;
    while (!VL_TOKEN_COMPARE(current, "}")) {
        vl_css_rule_t rule = {0};
        if (parse_rule(parser, &rule)) {
            vl_css_rule_deinit(&rule);
            while (true) {
                if (VL_TOKEN_COMPARE(current, ";")) {
                    tokenize(parser);
                    break;
                }
                if (VL_TOKEN_COMPARE(current, "}")) {
                    tokenize(parser);
                    return VL_SUCCESS;
                }
                tokenize(parser);
            }
            continue;
        }
        if (VL_TOKEN_COMPARE(current, ";")) {
            if (tokenize(parser)) goto fail;
        }
        rule.priority = class_priority;
        for (int i = 0; i < VL_DA_LENGTH(class->rules); i++) {
            if (strcmp(class->rules[i].property, rule.property) == 0) {
                vl_css_rule_deinit(class->rules + i);
                VL_DA_DELETE(class->rules, i);
                break;
            }
        }
        VL_DA_APPEND(class->rules, rule);
    }
    VL_TOKEN_CONSUME(parser, "}", goto fail);

    return VL_SUCCESS;
    fail:
    vl_css_class_deinit(class);
    tokenize(parser); // skip faulty token to avoid infinite loops
    return VL_ERROR;
}

vl_result_t vl_css_parser_deinit(vl_css_parser_t *parser) {
    if (!parser) return VL_ERROR;
    vl_css_lexer_deinit(&parser->lexer);
    return VL_SUCCESS;
}