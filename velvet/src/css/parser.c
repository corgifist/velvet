#include "velvet/css/parser.h"
#include "css/lexer.h"
#include "css/style.h"
#include "support/da.h"
#include "support/memory.h"
#include "support/result.h"
#include "support/str.h"
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
    return VL_SUCCESS;
}

static vl_css_size_metric_type_t map_str_to_metric_type(const char *str) {
    static const struct {
        const char *unit;
        vl_css_size_metric_type_t type;
    } s_metric_unit_map[] = {
        "px", VL_CSS_SIZE_METRIC_PIXELS
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
    vl_css_size_metric_type_t metric_type = map_str_to_metric_type(current->text);
    if (tokenize(parser) || metric_type == VL_CSS_SIZE_METRIC_NONE) return VL_CSS_VALUE_NONE();
    return VL_CSS_VALUE_METRIC1(
        VL_CSS_SIZE_METRIC(metric_type, value)
    );
}

static vl_css_value_t parse_primary_value(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    vl_css_token_t *current = parser->lookahead;
    if (current->type == VL_CSS_TOKEN_TYPE_NUMBER && (current + 1)->type == VL_CSS_TOKEN_TYPE_ID) {
        // single metric: 10px / 5em / 25%
        return parse_single_metric(parser, rule);
    }

    fail:
    return VL_CSS_VALUE_NONE();

}

static vl_css_value_t parse_shorthand_metric4(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    vl_css_size_metric_t max_metric[4];
    int metric_count = 0;

    vl_css_token_t *current = parser->lookahead;
    for (int i = 0; i < VL_ARR_LEN(max_metric); i++) {
        if (current->type != VL_CSS_TOKEN_TYPE_NUMBER || (current + 1)->type != VL_CSS_TOKEN_TYPE_ID)
            break;
        vl_css_value_t single_metric = parse_single_metric(parser, rule);
        if (single_metric.type != VL_CSS_VALUE_SIZE_METRIC1) {
            return VL_CSS_VALUE_NONE();
        }
        vl_css_size_metric_t metric_value = single_metric.as.metric1;
        max_metric[metric_count++] = metric_value;
    }

    switch (metric_count) {
    case 1: return VL_CSS_VALUE_METRIC1(max_metric[0]);
    case 4: return VL_CSS_VALUE_METRIC4(max_metric[0], max_metric[1], max_metric[2], max_metric[3]);
    }

    return VL_CSS_VALUE_NONE();
}

typedef vl_css_value_t (*vl_parse_value_with_context)(vl_css_parser_t *parser, vl_css_rule_t *rule);
static const struct {
    const char *property;
    vl_parse_value_with_context parse_with_context;
} s_context_table[] = {
    {"padding", parse_shorthand_metric4}
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
    rule->property = VL_DA_INIT_WITH_CAPACITY(char, current->text_length + 1);
    memcpy(rule->property, current->text, current->text_length);
    rule->property[current->text_length] = '\0';
    if (tokenize(parser)) goto fail;
    VL_TOKEN_CONSUME(parser, ":", goto fail);
    rule->value = dispatch_parse_value(parser, rule);

    return VL_SUCCESS;
    fail:
    vl_css_rule_deinit(rule);
    return VL_ERROR;
}

vl_result_t vl_css_parser_get(vl_css_parser_t *parser, vl_css_class_t *class) {
    if (!parser) return VL_ERROR;
    vl_css_token_t *current = parser->lookahead;
    if (current->type == VL_CSS_TOKEN_TYPE_STOP) {
        return VL_STOP;
    }
    class->name = VL_DA_INIT_WITH_CAPACITY(char, current->text_length + 1);
    memcpy(class->name, current->text, current->text_length);
    class->name[current->text_length] = '\0';
    if (tokenize(parser)) goto fail;
    VL_TOKEN_CONSUME(parser, "{", goto fail);
    class->rules = VL_DA_INIT(vl_css_rule_t);
    while (!VL_TOKEN_COMPARE(current, "}")) {
        vl_css_rule_t rule = {0};
        if (parse_rule(parser, &rule)) goto fail;
        if (VL_TOKEN_COMPARE(current, ";")) {
            if (tokenize(parser)) goto fail;
        }
        VL_DA_APPEND(class->rules, rule);
    }
    VL_TOKEN_CONSUME(parser, "}", goto fail);

    return VL_SUCCESS;
    fail:
    vl_css_class_deinit(class);

    return VL_ERROR;
}

vl_result_t vl_css_parser_deinit(vl_css_parser_t *parser) {
    if (!parser) return VL_ERROR;
    vl_css_lexer_deinit(&parser->lexer);
    return VL_SUCCESS;
}