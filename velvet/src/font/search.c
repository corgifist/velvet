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

typedef struct {
    const char *ptr;
    int len;
} font_root;

static int is_space(unsigned char c)
{
    return c == ' ' || c == '\t' ||
           c == '\n' || c == '\r' ||
           c == '\f' || c == '\v';
}

static int equals_word(const char *s, size_t len,
                       const char *word, size_t word_len)
{
    return len == word_len && utf8ncasecmp(s, word, word_len) == 0;
}

font_root font_root_name(const char *description, size_t description_size) {
#define STR_LEN(STR) { STR, sizeof(STR) - 1 }
    static const struct {
        const char *text;
        size_t len;
    } suffixes[] = {
        STR_LEN("Regular"),
        STR_LEN("Italic"),
        STR_LEN("Bold"),
        STR_LEN("Condensed"),
        STR_LEN("Narrow"),
        STR_LEN("Light"),
        STR_LEN("Oblique"),
        STR_LEN("Thin"),
        STR_LEN("Medium"),
        STR_LEN("UltraLight"),
        STR_LEN("Black"),
        STR_LEN("UltraBlack"),
        STR_LEN("ExtraLight"),
        STR_LEN("ExtraBlack"),
    };

    font_root result = { NULL, 0 };

    if (description == NULL || description_size == 0)
        return result;

    size_t length = 0;
    while (length < description_size && description[length] != '\0')
        ++length;

    size_t start = 0;
    while (start < length &&
           is_space((unsigned char)description[start])) {
        ++start;
    }

    size_t end = length;
    while (end > start &&
           is_space((unsigned char)description[end - 1])) {
        --end;
    }

    result.ptr = description;
    result.len = length;

    for (;;) {
        size_t word_start = end;

        while (word_start > start &&
               !is_space((unsigned char)description[word_start - 1])) {
            --word_start;
        }

        const char *word = description + word_start;
        size_t word_len = end - word_start;
        int is_style_word = 0;

        for (size_t i = 0; i < VL_ARR_LEN(suffixes); i++) {
            if (equals_word(word, word_len, suffixes[i].text, suffixes[i].len)) {
                is_style_word = 1;
                break;
            }
        }

        if (!is_style_word)
            break;

        end = word_start;

        while (end > start &&
               is_space((unsigned char)description[end - 1])) {
            --end;
        }
    }

    result.ptr = description + start;
    result.len = end - start;
    return result;
}

bool vl_font_search_compare_family_names(const char *family_name, const char *query) {
    if (!family_name || !query) return false;
    while (*query != '\0' && *query == ' ') { query++; }
    int family_len = strlen(family_name);
    int query_len = strlen(query);
    if (query_len > family_len) return false;
    while (query[query_len - 1] == ' ' && query_len > 0) { query_len--; }
    if (query_len == 0) return false;

    font_root family_root = font_root_name(family_name, family_len);
    font_root query_root = font_root_name(query, query_len);
    return (query_root.len == family_root.len && utf8ncasecmp(family_root.ptr, query_root.ptr, query_root.len) == 0);
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

vl_result_t vl_font_search_description_print(const vl_font_search_description_t *search) {
    if (!search) return VL_ERROR;
    printf("font %s at path %s", search->name, search->path);
    if (search->subfonts) {
        int len = VL_DA_LENGTH(search->subfonts);
        if (len == 1) {
            printf(" at index %i", search->subfonts[0]);
        } else if (len > 1) {
            printf(" at indices ");
            for (int i = 0; i < len; i++) {
                printf("%i", i);
                if (i != len - 1) printf(", ");
            }
        }
    }
    printf("\n");
    return VL_SUCCESS;
}

vl_result_t vl_font_search_description_deinit(vl_font_search_description_t *search) {
    if (!search) return VL_ERROR;
    VL_STRING_FREE(search->name);
    VL_STRING_FREE(search->path);
    VL_DA_FREE(search->subfonts);
    return VL_SUCCESS;
}