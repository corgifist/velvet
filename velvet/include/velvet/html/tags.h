#ifndef VELVET_HTML_TAGS_H
#define VELVET_HTML_TAGS_H

#include "velvet/common.h"
#include "velvet/support/api.h"

static const char *s_html_inline_tags[] = {
    "span",
    "code",
    "text"
};

VL_API bool vl_html_is_tag_inline_ex(const char *tag_begin, const char *tag_end);
VL_API bool vl_html_is_tag_inline(const char *tag);

#endif // VELVET_HTML_TAGS_H