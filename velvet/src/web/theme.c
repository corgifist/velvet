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

    center {
        text-align: center;
    }

    h1 {
        display: block;
        font-weight: bold;
        font-size: 2em;
        margin-block-start: 0.67em;
        margin-block-end: 0.67em;
        margin-inline-start: 0;
        margin-inline-end: 0;
    }

    h2 {
        display: block;
        font-weight: bold;
        font-size: 1.5em;
        margin-block-start: 0.83em;
        margin-block-end: 0.83em;
        margin-inline-start: 0;
        margin-inline-end: 0;
    }

    h3 {
        display: block;
        font-weight: bold;
        font-size: 1.17em;
        margin-block-start: 1em;
        margin-block-end: 1em;
        margin-inline-start: 0;
        margin-inline-end: 0;
    }

    h4 {
        display: block;
        font-weight: bold;
        font-size: 1em;
        margin-block-start: 1.33em;
        margin-block-end: 1.33em;
        margin-inline-start: 0;
        margin-inline-end: 0;
    }

    h5 {
        display: block;
        font-weight: bold;
        font-size: 0.83em;
        margin-block-start: 1.67em;
        margin-block-end: 1.67em;
        margin-inline-start: 0;
        margin-inline-end: 0;
    }

    h6 {
        display: block;
        font-weight: bold;
        font-size: 0.67em;
        margin-block-start: 2.33em;
        margin-block-end: 2.33em;
        margin-inline-start: 0;
        margin-inline-end: 0;
    }

    hr {
        display: block;
        color: gray;
        border-style: inset;
        border-width: 1px;
        margin-block-start: 0.5em;
        margin-block-end: 0.5em;
        margin-inline-start: auto;
        margin-inline-end: auto;
        overflow: hidden;
    }
);

const char *vl_web_theme_default_stylesheet() {
    return s_default_stylesheet;
}