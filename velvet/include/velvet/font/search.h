#ifndef VELVET_FONT_SEARCH_H
#define VELVET_FONT_SEARCH_H

#include "velvet/support/api.h"
#include "velvet/support/da.h"
#include "velvet/support/result.h"

struct vl_font_search_description {
    VL_DA_STRING name;
    VL_DA_STRING path;
};

typedef struct vl_font_search_description vl_font_search_description_t;

VL_API vl_result_t vl_font_search_query(VL_DA(vl_font_search_description_t)* results, const char *name);

#endif // VELVET_FONT_SEARCH_H