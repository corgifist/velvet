#ifndef VELVET_WEB_THEME_H
#define VELVET_WEB_THEME_H

#include "velvet/css/stylesheet.h"
#include "velvet/support/api.h"
#include "velvet/support/color.h"

struct vl_web_theme {
    vl_color_t canvas_text;
};

typedef struct vl_web_theme vl_web_theme_t;

VL_API bool vl_web_theme_supports_property(const char *property);
VL_API vl_color_t vl_web_theme_get_property(const vl_web_theme_t *theme, const char *property, vl_color_t fallback);

VL_API const vl_web_theme_t *vl_web_theme_default();
VL_API const char *vl_web_theme_default_stylesheet();

#endif // VELVET_WEB_THEME_H