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