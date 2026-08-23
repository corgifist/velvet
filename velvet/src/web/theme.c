#include "velvet/web/theme.h"
#include "css/stylesheet.h"
#include "support/str.h"

static const char *s_web_theme_properties[] = {
    "canvastext"
};

bool vl_web_theme_supports_property(const char *property) {
    if (!property) return false;
    for (int i = 0; i < VL_ARR_LEN(s_web_theme_properties); i++) {
        if (strcmp(property, s_web_theme_properties[i]) == 0) return true;
    }
    return false;
}

vl_color_t vl_web_theme_get_property(const vl_web_theme_t *theme, const char *property, vl_color_t fallback) {
    if (!theme || !property) return fallback;
    if (strcmp(property, "canvastext") == 0) return theme->canvas_text;
    return fallback;
}

static const vl_web_theme_t s_default_theme = {
    .canvas_text = VL_COLOR(0, 0, 0, 1)
};

const vl_web_theme_t *vl_web_theme_default() {
    return &s_default_theme;
}

static const char *s_default_stylesheet = VL_STRINGIFY(
    html {
        display: block;
        color: canvastext;
    }

    body {
        display: block;
        margin: 8px;
    }

    p {
        display: block;
        margin-block-start: 1em;
        margin-block-end: 1em;
    }

    style {
        display: none;
    }
);

const char *vl_web_theme_default_stylesheet() {
    return s_default_stylesheet;
}