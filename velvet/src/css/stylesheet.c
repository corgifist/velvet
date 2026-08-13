#include "velvet/css/stylesheet.h"
#include "css/style.h"
#include "css/parser.h"
#include "support/result.h"

vl_result_t vl_css_stylesheet_init_(vl_css_stylesheet_t *stylesheet, const char *text, vl_source_location_t loc) {
    if (!stylesheet) return VL_ERROR;
    vl_css_stylesheet_init_empty(stylesheet, loc);
    vl_css_parser_t parser = {0};
    vl_css_parser_init(&parser, text, loc);
    vl_css_class_t class = {0};
    while (vl_css_parser_get(&parser, &class) != VL_STOP) {
        VL_DA_APPEND(stylesheet->classes, class);
    }
    vl_css_parser_deinit(&parser);
    return VL_SUCCESS;
}

vl_result_t vl_css_stylesheet_init_empty_(vl_css_stylesheet_t *stylesheet, vl_source_location_t loc) {
    if (!stylesheet) return VL_ERROR;
    stylesheet->classes = VL_DA_INIT(vl_css_class_t);
    return VL_SUCCESS;
}

vl_result_t vl_css_stylesheet_merge(vl_css_stylesheet_t *dst, const vl_css_stylesheet_t *sheet) {
    if (!dst || !sheet) return VL_ERROR;
    for (int i = 0; i < VL_DA_LENGTH(sheet->classes); i++) {
        vl_css_class_t *class = sheet->classes + i;
        vl_css_class_t *duplicate_class = NULL;
        for (int j = 0; j < VL_DA_LENGTH(dst->classes); j++) {
            vl_css_class_t *dst_class = dst->classes + j;
            if (strcmp(class->name, dst_class->name) == 0) {
                duplicate_class = dst_class;
                break;
            }
        }
        if (duplicate_class) {
            vl_css_class_merge(duplicate_class, class);
        } else {
            vl_css_class_t class_copy = {0};
            vl_css_class_copy(&class_copy, class);
            VL_DA_APPEND(dst->classes, class_copy);
        }
    }

    return VL_SUCCESS;
}

vl_css_class_t *vl_css_stylesheet_find_class(vl_css_stylesheet_t *stylesheet, const char *class) {
    if (!stylesheet || !class) return NULL;
    if (stylesheet->classes) {
        for (int i = 0; i < VL_DA_LENGTH(stylesheet->classes); i++) {
            vl_css_class_t *c = stylesheet->classes + i;
            if (c->name && strcmp(c->name, class) == 0) return c;
        }
    }
    return NULL;
}

vl_result_t vl_css_stylesheet_deinit(vl_css_stylesheet_t *stylesheet) {
    if (!stylesheet) return VL_ERROR;
    if (stylesheet->classes) {
        for (int i = 0; i < VL_DA_LENGTH(stylesheet->classes); i++) {
            vl_css_class_deinit(stylesheet->classes + i);
        }
    }
    VL_DA_FREE(stylesheet->classes);
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