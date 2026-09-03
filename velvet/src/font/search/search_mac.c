#include "velvet/support/result.h"
#include "velvet/support/da.h"
#include "velvet/font/search.h"
#include "vendor/utf8.h"

#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CoreText.h>

static VL_DA_STRING cf_string_to_da_string(CFStringRef cf_string) {
    size_t cf_length = CFStringGetLength(cf_string);
    CFIndex buffer_length = 0;
    CFStringGetBytes(
        cf_string, CFRangeMake(0, cf_length), 
        kCFStringEncodingUTF8, 0, false, 
        NULL, 0, &buffer_length);
    VL_DA_STRING result = VL_DA_INIT_WITH_CAPACITY(char, buffer_length + 1);
    CFStringGetCString(cf_string, result, buffer_length + 1, kCFStringEncodingUTF8);
    result[buffer_length] = '\0';
    return result;
}

static bool families_match(VL_DA_STRING family_name, const char *query) {
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

vl_result_t vl_font_search_query(VL_DA(vl_font_search_description_t)* results, const char *name) {
    if (!results || !name) return VL_ERROR;
    CTFontCollectionRef font_collection = CTFontCollectionCreateFromAvailableFonts(NULL);
    if (!font_collection) {
        return VL_ERROR;
    }
    CFArrayRef fonts = CTFontCollectionCreateMatchingFontDescriptors(font_collection);
    if (!fonts) {
        CFRelease(font_collection);
        return VL_ERROR;
    }
    if (!*results) {
        *results = VL_DA_INIT(vl_font_search_description_t);
    }

    CFIndex count = CFArrayGetCount(fonts);
    for (CFIndex i = 0; i < count; i++) {
        CTFontDescriptorRef font = (CTFontDescriptorRef) CFArrayGetValueAtIndex(fonts, i);
        CFStringRef font_name = (CFStringRef) CTFontDescriptorCopyAttribute(font, kCTFontDisplayNameAttribute);
        CFURLRef font_path_url = (CFURLRef) CTFontDescriptorCopyAttribute(font, kCTFontURLAttribute);
        CFStringRef font_path_string = CFURLCopyFileSystemPath(font_path_url, kCFURLPOSIXPathStyle);
        vl_font_search_description_t desc = {0};
        if (font_name) {
            desc.name = cf_string_to_da_string(font_name);
            if (!families_match(desc.name, name)) {
                VL_DA_FREE(desc.name);
                goto release;
            }
        }
        if (font_path_string) {
            desc.path = cf_string_to_da_string(font_path_string);
        }
        VL_DA_APPEND(*results, desc);
        release:
        if (font_name) CFRelease(font_name);
        if (font_path_url) CFRelease(font_path_url);
        if (font_path_string) CFRelease(font_path_string);
    }

    CFRelease(fonts);
    CFRelease(font_collection);
    return VL_SUCCESS;
}