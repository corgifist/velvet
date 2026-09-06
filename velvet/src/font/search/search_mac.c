#include "velvet/support/result.h"
#include "velvet/support/da.h"
#include "velvet/font/search.h"
#include "velvet/support/platform.h"

#if VL_PLATFORM(MAC)

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

vl_result_t vl_font_search_query(VL_DA(vl_font_search_description_t)* results, const char *name) {
    if (!results) return VL_ERROR;
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
            if ((name && !vl_font_search_compare_family_names(desc.name, name))) {
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

#endif