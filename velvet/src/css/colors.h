#ifndef VELVET_CSS_COLORS_H
#define VELVET_CSS_COLORS_H

#include "velvet/css/style.h"

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
    {"highlight-red", VL_CSS_VALUE_RGBA(255, 0, 0, 0.1)},
    {"highlight-green", VL_CSS_VALUE_RGBA(0, 255, 0, 0.1)},
    {"highlight-blue", VL_CSS_VALUE_RGBA(0, 0, 255, 0.1)},
    {"highlight-orange", VL_CSS_VALUE_RGBA(255, 165, 0, 0.1)},
    {"darkorange", VL_CSS_VALUE_RGBA(255, 140, 0, 1)},
    {"orange", VL_CSS_VALUE_RGBA(255, 165, 0, 1)}
};

#endif // VELVET_CSS_COLORS_H