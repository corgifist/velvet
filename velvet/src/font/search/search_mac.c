#include "velvet/support/result.h"
#include "velvet/support/da.h"
#include "velvet/font/search.h"
#include "velvet/support/platform.h"
#include "velvet/support/string.h"
#include "vendor/utf8.h"

#if VL_PLATFORM(MAC)

#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>

static VL_STRING cf_string_to_da_string(CFStringRef cf_string) {
    size_t cf_length = CFStringGetLength(cf_string);
    CFIndex buffer_length = 0;
    CFStringGetBytes(
        cf_string, CFRangeMake(0, cf_length), 
        kCFStringEncodingUTF8, 0, false, 
        NULL, 0, &buffer_length);
    VL_DA_STRING result = VL_DA_INIT(char, buffer_length + 1);
    CFStringGetCString(cf_string, result, buffer_length + 1, kCFStringEncodingUTF8);
    VL_DA_HEADER(result)->count = buffer_length;
    VL_STRING compact = VL_STRING_FROM_DA(result);
    VL_DA_FREE(result);
    return compact;
}

static VL_DA(int) get_subfonts(CTFontDescriptorRef descriptor, const char *path) {
    VL_DA(int) result = NULL;

    CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8*) path, strlen(path), false);

    CFArrayRef descriptors =
        CTFontManagerCreateFontDescriptorsFromURL(url);

    int count = descriptors ? CFArrayGetCount(descriptors) : 0;

    CFStringRef target_font_name =
        CTFontDescriptorCopyAttribute(descriptor, kCTFontNameAttribute);
    for (int i = 0; descriptors && i < count; i++) {
        CTFontDescriptorRef candidate =
            (CTFontDescriptorRef)CFArrayGetValueAtIndex(descriptors, i);

        CFStringRef candidate_font_name =
            CTFontDescriptorCopyAttribute(candidate, kCTFontNameAttribute);

        bool matches =
            (CFStringCompare(candidate_font_name, target_font_name, 0) == 0);

        // printf("%s == %s\n", cf_string_to_da_string(target_font_name), cf_string_to_da_string(candidate_font_name));
        if (candidate_font_name) CFRelease(candidate_font_name);

        if (matches) {
            if (!result) result = VL_DA_INIT(int, 1);
            VL_DA_APPEND(result, i);
        }
    }

    if (target_font_name) CFRelease(target_font_name);
    if (descriptors) CFRelease(descriptors);
    if (url) CFRelease(url);
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
                VL_STRING_FREE(desc.name);
                goto release;
            }
        }
        if (font_path_string) {
            desc.path = cf_string_to_da_string(font_path_string);
            desc.subfonts = get_subfonts(font, desc.path);
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