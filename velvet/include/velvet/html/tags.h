#ifndef VELVET_HTML_TAGS_H
#define VELVET_HTML_TAGS_H

#include "velvet/common.h"
#include "velvet/support/api.h"

static const char *s_html_inline_tags[] = {
    "span",
    "code",
    "text"
};

static const char *s_html_void_tags[] = {
    "meta",
    "link",
    "img",
    "hr"
};

VL_API bool vl_html_is_tag_inline(const char *tag);
VL_API bool vl_html_is_tag_inline_ex(const char *tag_begin, const char *tag_end);

VL_API bool vl_html_is_tag_void(const char *tag);
VL_API bool vl_html_is_tag_void_ex(const char *tag_begin, const char *tag_end);

#endif // VELVET_HTML_TAGS_H