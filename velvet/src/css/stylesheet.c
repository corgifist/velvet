#include "velvet/css/stylesheet.h"
#include "css/style.h"
#include "css/parser.h"
#include "support/result.h"
#include "support/math.h"

vl_result_t vl_css_stylesheet_init_(vl_css_stylesheet_t *stylesheet, const char *text, vl_source_location_t loc) {
    if (!stylesheet) return VL_ERROR;
    vl_css_stylesheet_init_empty(stylesheet, loc);
    vl_css_parser_t parser = {0};
    vl_css_parser_init(&parser, text, loc);
    vl_css_class_t class = {0};
    vl_result_t result;
    while ((result = vl_css_parser_get(&parser, &class)) != VL_STOP) {
        if (result == VL_SUCCESS) {
            vl_css_stylesheet_add_class(stylesheet, &class);
        }
        class = (vl_css_class_t) {0};
    }
    vl_css_parser_deinit(&parser);
    return VL_SUCCESS;
}

vl_result_t vl_css_stylesheet_init_empty_(vl_css_stylesheet_t *stylesheet, vl_source_location_t loc) {
    if (!stylesheet) return VL_ERROR;
    stylesheet->classes = VL_DA_INIT(vl_css_class_t);
    stylesheet->max_priority = 1;
    return VL_SUCCESS;
}

vl_result_t vl_css_stylesheet_add_class(vl_css_stylesheet_t *stylesheet, vl_css_class_t *class) {
    if (!stylesheet || !class) return VL_ERROR;
    if (stylesheet->classes) {
        vl_css_class_t tmp_class = *class;
        if (tmp_class.rules) {
            int priority = stylesheet->max_priority++;
            for (int i = 0; i < VL_DA_LENGTH(tmp_class.rules); i++) {
                tmp_class.rules[i].priority = priority;
            }
        }
        VL_DA_APPEND(stylesheet->classes, tmp_class);
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_stylesheet_merge(vl_css_stylesheet_t *dst, const vl_css_stylesheet_t *sheet) {
    if (!dst || !sheet) return VL_ERROR;
    if (sheet->classes) {
        for (int i = 0; i < VL_DA_LENGTH(sheet->classes); i++) {
            vl_css_class_t copy_class = {0};
            vl_css_class_copy(&copy_class, sheet->classes + i);
            vl_css_stylesheet_add_class(dst, &copy_class);
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_stylesheet_broad_query(vl_css_stylesheet_t *sheet, const vl_css_class_selector_t *target_selector, vl_css_class_t ***result) {
    if (!sheet || !target_selector || !result) return VL_ERROR;
    if (!*result) {
        *result = VL_DA_INIT(vl_css_class_t*);
    }
    for (int i = 0; i < VL_DA_LENGTH(sheet->classes); i++) {
        vl_css_class_t *class = sheet->classes + i;
        bool match = false;
        if (class->selectors) {
            for (int j = 0; j < VL_DA_LENGTH(class->selectors); j++) {
                vl_css_class_selector_t *selector = class->selectors + j;
                if (selector == target_selector) {
                    match = true;
                    break;
                }
                if (!selector->id_chain) continue;
                bool id_pass = true;
                size_t len = VL_MIN(VL_DA_LENGTH(selector->id_chain), VL_DA_LENGTH(target_selector->id_chain));
                for (int k = 0; k < len; k++) {
                    vl_css_class_id_t *id = selector->id_chain + k;
                    vl_css_class_id_t *target_id = target_selector->id_chain + k;
                    if (id->type != target_id->type) {
                        id_pass = false;
                        break;
                    }
                    if (id->name && target_id->name && strcmp(id->name, target_id->name) != 0) {
                        id_pass = false;
                        break;
                    }
                }
                if (id_pass) {
                    match = true;
                    break;
                }
            }
        }
        if (match) {
            VL_DA_APPEND(*result, class);
        }
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_stylesheet_deinit(vl_css_stylesheet_t *stylesheet) {
    if (!stylesheet) return VL_ERROR;
    if (stylesheet->classes) {
        for (int i = 0; i < VL_DA_LENGTH(stylesheet->classes); i++) {
            vl_css_class_deinit(stylesheet->classes + i);
        }
        VL_DA_FREE(stylesheet->classes);
    }
    return VL_SUCCESS;
}

vl_result_t vl_css_stylesheet_print(vl_css_stylesheet_t *stylesheet) {
    if (!stylesheet) return VL_ERROR;
    if (stylesheet->classes) {
        size_t len = VL_DA_LENGTH(stylesheet->classes);
        for (int i = 0; i < len; i++) {
            vl_css_class_print(stylesheet->classes + i);
            if (i != len - 1) {
                printf("\n");
            }
        }
    }
    return VL_SUCCESS;
}