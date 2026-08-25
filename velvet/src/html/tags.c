#include "velvet/html/tags.h"
#include "velvet/support/memory.h"

bool vl_html_is_tag_inline(const char *tag) {
    if (!tag) return false;
    return vl_html_is_tag_inline_ex(tag, tag + strlen(tag));
}

bool vl_html_is_tag_inline_ex(const char *tag_begin, const char *tag_end) {
    if (!tag_begin || !tag_end) return false;
    int len = tag_end - tag_begin;
    for (int i = 0; i < VL_ARR_LEN(s_html_inline_tags); i++) {
        int canditate_len = strlen(s_html_inline_tags[i]);
        if (len != canditate_len) continue;
        if (memcmp(s_html_inline_tags[i], tag_begin, len) == 0) return true;
    }
    return false;
}