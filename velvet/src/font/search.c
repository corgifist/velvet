#include "velvet/font/search.h"
#include "support/result.h"
#include "velvet/support/platform.h"

#if VL_PLATFORM(MAC)
    #include "search/search_mac.c"
#elif VL_PLATFORM(WINDOWS)
    #include "search/search_win.c"
#endif

#include "vendor/utf8.h"
#include "velvet/web/fonts.h"

bool vl_font_search_compare_family_names(const char *family_name, const char *query) {
    if (!family_name || !query) return false;
    while (*query != '\0' && *query == ' ') { query++; }
    int family_len = strlen(family_name);
    int query_len = strlen(query);
    // printf("query: %s\n", query);
    if (query_len > family_len) return false;
    while (query[query_len - 1] == ' ' && query_len > 0) { query_len--; }
    if (query_len == 0) return false;
    // if (memcmp(family_name, query, query_len) == 0) printf("compare: '%s' '%s' %i res: %i\n", family_name, query, query_len, utf8ncasecmp(family_name, query, query_len));
    return (utf8ncasecmp(family_name, query, query_len) == 0);
}

vl_result_t vl_font_search_classify(const char *font_name, int *weight, bool *italic, bool *bold, bool *oblique, bool *narrow) {
    if (!font_name) return VL_ERROR;
    if (weight) {
        *weight = VL_WEB_FONT_REGULAR;
        if (utf8casestr(font_name, "Light")) *weight = VL_WEB_FONT_LIGHT;
        if (utf8casestr(font_name, "ExtraLight") || utf8casestr(font_name, "Extra Light")) 
            *weight = VL_WEB_FONT_EXTRA_LIGHT;
        if (utf8casestr(font_name, "Bold")) *weight = VL_WEB_FONT_BOLD;
        if (utf8casestr(font_name, "Black")) *weight = VL_WEB_FONT_BLACK;
    }
    if (italic) {
        *italic = (utf8casestr(font_name, "Italic") != 0);
    }
    if (bold) {
        *bold = (utf8casestr(font_name, "Bold") != 0);
    }
    if (oblique) {
        *oblique = (utf8casestr(font_name, "Oblique") != 0);
    }
    if (narrow) {
        *narrow = (utf8casestr(font_name, "Narrow") != 0
                    || utf8casestr(font_name, "Condensed") != 0 
                    || utf8casestr(font_name, "Compressed") != 0);
    }
    return VL_SUCCESS;
}