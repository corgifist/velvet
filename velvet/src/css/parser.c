#include "velvet/css/parser.h"
#include "css/lexer.h"
#include "css/style.h"
#include "support/da.h"
#include "support/memory.h"
#include "support/result.h"
#include "support/string.h"
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
        if (tokenize(PARSER) || skip_spaces(parser)) { \
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

#define VL_TOKEN_EMPTY(TOKEN) \
    VL_TOKEN_COMPARE(TOKEN, " ") || VL_TOKEN_COMPARE(TOKEN, "\t") || VL_TOKEN_COMPARE(TOKEN, "\n")

static vl_result_t skip_spaces(vl_css_parser_t *parser) {
    vl_css_token_t *current = parser->lookahead;
    while (VL_TOKEN_EMPTY(current)) {
        // skip all meaningless tokens
        if (tokenize(parser)) return VL_ERROR;
    } 
    return VL_SUCCESS;
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
    if (VL_TOKEN_COMPARE(current, "auto")) {
        tokenize(parser); skip_spaces(parser);
        return VL_CSS_VALUE_METRIC1(VL_CSS_SIZE_AUTO());
    }
    float value = strtod(current->text, NULL);
    if (tokenize(parser) || skip_spaces(parser)) return VL_CSS_VALUE_NONE();
    if (current->type != VL_CSS_TOKEN_TYPE_ID || VL_TOKEN_COMPARE(current, "auto")) {
        return VL_CSS_VALUE_METRIC1(
            VL_CSS_SIZE_PIXELS(value)
        );
    }
    vl_css_size_metric_type_t metric_type = map_str_to_metric_type(current->text);
    if (tokenize(parser) || skip_spaces(parser) || metric_type == VL_CSS_SIZE_METRIC_NONE) return VL_CSS_VALUE_NONE();
    if (metric_type == VL_CSS_SIZE_METRIC_PERCENTAGE) metric_type /= 100.0f;
    return VL_CSS_VALUE_METRIC1(
        VL_CSS_SIZE_METRIC(metric_type, value)
    );
}

static VL_STRING parse_id_or_string(vl_css_parser_t *parser) {
    vl_css_token_t *current = parser->lookahead;
    if (current->type == VL_CSS_TOKEN_TYPE_ID || current->type == VL_CSS_TOKEN_TYPE_STRING) {
        const char *begin = current->text;
        int len = current->text_length;
        if (current->type == VL_CSS_TOKEN_TYPE_STRING) {
            begin++;
            len -= 2;
        }
        VL_STRING result = VL_STRING_INIT(begin, len);
        tokenize(parser); skip_spaces(parser);
        return result;
    }
    return NULL;
}

#include "colors.h"

static vl_css_value_t parse_generic_color(vl_css_parser_t *parser, vl_css_rule_t *rule, int max_components) {
    if (tokenize(parser) || skip_spaces(parser) || tokenize(parser) || skip_spaces(parser)) goto fail; // skip 'rgb' / 'rgba' and '('
    vl_css_token_t *current = parser->lookahead;
    float components[4] = {0.0, 0.0, 0.0, 1.0};
    int component = 0;
    while (!VL_TOKEN_COMPARE(current, ")")) {
        if (component >= 4) goto fail;
        if (current->type != VL_CSS_TOKEN_TYPE_NUMBER) goto fail;
        float value = strtod(current->text, NULL);
        components[component++] = value;
        if (tokenize(parser) || skip_spaces(parser)) goto fail; // skip the number
        if (VL_TOKEN_COMPARE(current, ",") || VL_TOKEN_COMPARE(current, "/")) {
            if (tokenize(parser) || skip_spaces(parser)) goto fail; // skip possible delimiter
        }
    }
    if (tokenize(parser) || skip_spaces(parser)) goto fail; // skip ')'
    switch (max_components) {
        case 4: return VL_CSS_VALUE_RGBA(components[0], components[1], components[2], components[3]);
    }
    fail:
    return VL_CSS_VALUE_NONE();
}

#include "constants.h"

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
                tokenize(parser); skip_spaces(parser);
                return s_const_literals[i];
            }
        }
    }
    return NULL;
}

static VL_STRING process_string(const char *begin, int len) {
    VL_STRING result = VL_DA_INIT(char);
    for (int i = 0; i < len; i++) {
        char c = begin[i];
        if (c == '\\' && i != len - 1) continue;
        *VL_DA_PUSH(result, char) = c;
    }
    *VL_DA_PUSH(result, char) = '\0';
    VL_STRING compact = VL_STRING_INIT(result);
    VL_DA_FREE(result);
    return compact;
}

static vl_css_value_t parse_primary_value(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    vl_css_token_t *current = parser->lookahead;
    if (current->type == VL_CSS_TOKEN_TYPE_ID) {
        for (int i = 0; i < VL_ARR_LEN(s_css_constants); i++) {
            if (current->text_length == strlen(s_css_constants[i].name) && 
                    vl_nstrcicmp(current->text, s_css_constants[i].name, current->text_length) == 0) {
                if (tokenize(parser) || skip_spaces(parser)) return VL_CSS_VALUE_NONE();
                return s_css_constants[i].value;
            }
        }
    }
    if (current->type == VL_CSS_TOKEN_TYPE_HEX_ID) {
        vl_css_value_t result = VL_CSS_VALUE_NONE();
        // printf("current: '%.*s', %i\n", current->text_length, current->text, current->text_length);
        switch (current->text_length) {
        case 3: {
            unsigned int r, g, b;
            if (sscanf(current->text, "%1x%1x%1x", &r, &g, &b) != 3) result = VL_CSS_VALUE_NONE();
            else result = VL_CSS_VALUE_RGBA(r * 17, g * 17, b * 17, 1);
            break;
        }
        case 4: {
            unsigned int r, g, b, a;
            if (sscanf(current->text, "%1x%1x%1x%1x", &r, &g, &b, &a) != 4) result = VL_CSS_VALUE_NONE();
            else result = VL_CSS_VALUE_RGBA(r * 17, g * 17, b * 17, a * 17);
            break;
        }
        case 6: {
            unsigned int r, g, b;
            if (sscanf(current->text, "%02x%02x%02x", &r, &g, &b) != 3) result = VL_CSS_VALUE_NONE();
            else result = VL_CSS_VALUE_RGBA(r, g, b, 1);
            break;
        }
        case 8: {
            unsigned int r, g, b, a;
            if (sscanf(current->text, "%02x%02x%02x%02x", &r, &g, &b, &a) != 4) result = VL_CSS_VALUE_NONE();
            else result = VL_CSS_VALUE_RGBA(r, g, b, a);
            break;
        }
        default: result = VL_CSS_VALUE_NONE(); break;
        }
        // printf("hex color\n");
        tokenize(parser); skip_spaces(parser);
        return result;
    }
    if (current->type == VL_CSS_TOKEN_TYPE_STRING) {
        vl_css_value_t result = VL_CSS_VALUE_STRING(process_string(current->text + 1, current->text_length - 2));
        tokenize(parser); skip_spaces(parser);
        return result;
    }
    if ((current->type == VL_CSS_TOKEN_TYPE_NUMBER && (current + 1)->type == VL_CSS_TOKEN_TYPE_ID)
            || VL_TOKEN_COMPARE(current, "auto")) {
        // single metric: 10px / 5em / 25%
        return parse_single_metric(parser, rule);
    }
    if (current->type == VL_CSS_TOKEN_TYPE_NUMBER && VL_TOKEN_COMPARE(current + 1, ";")) {
        bool dot_found = false;
        for (int i = 0; i < current->text_length; i++) {
            if (current->text[i] == '.') {
                dot_found = true;
                break;
            }
        }
        if (!dot_found) {
            char *endptr;
            int integer = strtol(current->text, &endptr, 10);
            tokenize(parser); skip_spaces(parser);
            return VL_CSS_VALUE_INTEGER(integer);
        }
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
        if (current->type != VL_CSS_TOKEN_TYPE_NUMBER && !VL_TOKEN_COMPARE(current, "auto"))
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
    if ((current->type == VL_CSS_TOKEN_TYPE_ID || current->type == VL_CSS_TOKEN_TYPE_STRING) && VL_TOKEN_COMPARE(current + 1, ";")) {
        bool is_string = (current->type == VL_CSS_TOKEN_TYPE_STRING);
        const char *literal = VL_STRING_INIT(current->text + is_string, current->text_length - is_string - is_string);
        tokenize(parser); skip_spaces(parser);
        return VL_CSS_VALUE_DYNAMIC_LITERAL(literal);
    }
    vl_css_value_t result = {.type = VL_CSS_VALUE_FONT_LIST, .as = {0}};
    vl_css_font_list_t *font_list = &result.as.font_list;
    font_list->fonts = VL_DA_INIT(VL_STRING);
    while (!VL_TOKEN_COMPARE(current, ";") && current->type != VL_CSS_TOKEN_TYPE_STOP) {
        VL_STRING id = parse_id_or_string(parser);
        if (id) *VL_DA_PUSH(font_list->fonts, VL_STRING) = id;
        else if (tokenize(parser) || skip_spaces(parser)) break;
        goto next;
        next:
        if (VL_TOKEN_COMPARE(current, ",")) {
            if (tokenize(parser) || skip_spaces(parser)) break;
        }
    }
    return result;
}

static vl_css_value_t parse_list(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    vl_css_value_t result = VL_CSS_VALUE_LIST();
    vl_css_token_t *current = parser->lookahead;
    while (!VL_TOKEN_COMPARE(current, ";")) {
        vl_css_value_t value = parse_primary_value(parser, rule);
        if (VL_TOKEN_COMPARE(current, ",")) {
            tokenize(parser); skip_spaces(parser);
        }
        if (value.type == VL_CSS_VALUE_NONE) {
            tokenize(parser); skip_spaces(parser);
            continue;
        }
        VL_DA_APPEND(result.as.list, value);
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
    {"font-family", parse_font_list},
    {"border", parse_list},
    {"border-style", parse_list},
    {"border-top", parse_list},
    {"border-right", parse_list},
    {"border-bottom", parse_list},
    {"border-left", parse_list},
    {"border-color", parse_list},
    {"border-width", parse_list},
    {"background", parse_list},
    {"inset", parse_shorthand_metric4}
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
    rule->property = VL_STRING_INIT(current->text, current->text_length);
    if (tokenize(parser) || skip_spaces(parser)) goto fail;
    VL_TOKEN_CONSUME(parser, ":", goto fail);
    rule->value = dispatch_parse_value(parser, rule);
    if (rule->value.type == VL_CSS_VALUE_NONE) goto fail;
    if (VL_TOKEN_COMPARE(current, "!") && VL_TOKEN_COMPARE(current + 1, "important")) {
        if (tokenize(parser) || skip_spaces(parser) || tokenize(parser) || skip_spaces(parser)) goto fail;
        rule->important = true;
    }
    return VL_SUCCESS;
    fail:
    vl_css_rule_deinit(rule);
    return VL_ERROR;
}

static vl_result_t parse_class_atom(vl_css_parser_t *parser, vl_css_class_atom_t *atom) {
    vl_css_token_t *current = parser->lookahead;
    VL_ZERO_OUT(atom);
    if (VL_TOKEN_COMPARE(current, "*")) {
        atom->type = VL_CSS_CLASS_ATOM_ALL;
        if (tokenize(parser)) goto fail;
        return VL_SUCCESS;
    }
    if (current->type == VL_CSS_TOKEN_TYPE_ID) {
        atom->type = VL_CSS_CLASS_ATOM_ELEMENT;
        atom->as.string = VL_STRING_INIT(current->text, current->text_length);
        if (tokenize(parser)) goto fail; 
        return VL_SUCCESS;
    }
    if (current->type == VL_CSS_TOKEN_TYPE_HEX_ID) {
        atom->type = VL_CSS_CLASS_ATOM_UNIQUE_ID;
        atom->as.string = VL_STRING_INIT(current->text, current->text_length);
        if (tokenize(parser)) goto fail; // skip id
        return VL_SUCCESS;
    }
    if (VL_TOKEN_COMPARE(current, ".") && (current + 1)->type == VL_CSS_TOKEN_TYPE_ID) {
        atom->type = VL_CSS_CLASS_ATOM_CLASS_NAMES;
        while (VL_TOKEN_COMPARE(current, ".")) {
            tokenize(parser); // skip '.'
            if (current->type != VL_CSS_TOKEN_TYPE_ID) break;
            if (!atom->as.class_names) atom->as.class_names = VL_DA_INIT(VL_STRING);
            *VL_DA_PUSH(atom->as.class_names, VL_STRING) = VL_STRING_INIT(current->text, current->text_length);
            tokenize(parser); // skip id
        }
        return VL_SUCCESS;
    }
    if (VL_TOKEN_COMPARE(current, ":") && VL_TOKEN_COMPARE(current + 1, ":") && (current + 2)->type == VL_CSS_TOKEN_TYPE_ID) {
        tokenize(parser); tokenize(parser);
        atom->type = VL_CSS_CLASS_ATOM_PSEUDO_ELEMENT;
        atom->as.string = VL_STRING_INIT(current->text, current->text_length);
        if (tokenize(parser)) goto fail;
        return VL_SUCCESS;
    }

    fail:
    vl_css_class_atom_deinit(atom);
    return VL_ERROR;
}

static vl_result_t parse_class_selector(vl_css_parser_t *parser, vl_css_class_selector_t *selector) {
    if (!selector->hierarchy) {
        selector->hierarchy = VL_DA_INIT(vl_css_class_id_t);
    }
    vl_css_token_t *current = parser->lookahead;
    while (true) {
        if (VL_TOKEN_COMPARE(parser->lookahead, ",") || VL_TOKEN_COMPARE(parser->lookahead, "{")) break; // moving onto the next selector
        vl_css_class_id_t id = {0};
        id.atoms = VL_DA_INIT(vl_css_class_atom_t);
        while (!VL_TOKEN_COMPARE(current, " ") && !VL_TOKEN_COMPARE(current, ",") && !VL_TOKEN_COMPARE(current, "{")) {
            vl_css_class_atom_t atom = {0};
            if (parse_class_atom(parser, &atom) || atom.type == VL_CSS_CLASS_ATOM_NONE) goto fail;
            VL_DA_APPEND(id.atoms, atom);
        }
        skip_spaces(parser);
        VL_DA_APPEND(selector->hierarchy, id);
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
        if (VL_TOKEN_COMPARE(current, ",") && (tokenize(parser) || skip_spaces(parser))) goto fail;
    }
    return VL_SUCCESS;
    fail:
    vl_css_class_deinit(class);
    return VL_ERROR;
}

vl_result_t vl_css_parser_get(vl_css_parser_t *parser, vl_css_class_t *class) {
    if (!parser || !class) return VL_ERROR;
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
                    tokenize(parser); skip_spaces(parser);
                    break;
                }
                if (VL_TOKEN_COMPARE(current, "}")) {
                    tokenize(parser); skip_spaces(parser);
                    return VL_SUCCESS;
                }
                tokenize(parser); skip_spaces(parser);
            }
            continue;
        }
        if (VL_TOKEN_COMPARE(current, ";")) {
            if (tokenize(parser) || skip_spaces(parser)) goto fail;
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
    tokenize(parser); skip_spaces(parser); // skip faulty token to avoid infinite loops
    return VL_ERROR;
}

vl_result_t vl_css_parser_get_rule(vl_css_parser_t *parser, vl_css_rule_t *rule) {
    if (!parser || !rule) return VL_ERROR;
    vl_css_token_t *current = parser->lookahead;
    if (current->type == VL_CSS_TOKEN_TYPE_STOP) {
        return VL_STOP;
    }
    vl_result_t result = parse_rule(parser, rule);
    if (VL_TOKEN_COMPARE(current, ";") || result) { tokenize(parser); skip_spaces(parser); }
    return result;
}

vl_result_t vl_css_parser_deinit(vl_css_parser_t *parser) {
    if (!parser) return VL_ERROR;
    vl_css_lexer_deinit(&parser->lexer);
    return VL_SUCCESS;
}