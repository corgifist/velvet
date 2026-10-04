#ifndef VELVET_FONT_SEARCH_H
#define VELVET_FONT_SEARCH_H

#include "velvet/support/api.h"
#include "velvet/support/da.h"
#include "velvet/support/result.h"
#include "velvet/font/font.h"
#include "velvet/support/string.h"

struct vl_font_search_description {
    VL_STRING name;
    VL_STRING path;
    VL_DA(int) subfonts;
};

typedef struct vl_font_search_description vl_font_search_description_t;

VL_API vl_result_t vl_font_search_query(VL_DA(vl_font_search_description_t)* results, const char *name);
VL_API bool vl_font_search_compare_family_names(const char *family_name, const char *target_name);
VL_API vl_result_t vl_font_search_classify(const char *font_name, 
    int *weight, bool *italic, bool *bold, bool *oblique, bool *narrow);
VL_API vl_result_t vl_font_search_description_print(const vl_font_search_description_t *search);
VL_API vl_result_t vl_font_search_description_deinit(vl_font_search_description_t *search);

#endif // VELVET_FONT_SEARCH_H