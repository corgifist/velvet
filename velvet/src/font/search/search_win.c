#include "font/font.h"
#include "velvet/support/result.h"
#include "velvet/support/da.h"
#include "velvet/font/search.h"
#include "velvet/support/win32.h"
#include "velvet/support/platform.h"
#include "vendor/utf8.h"
#include <minwindef.h>

#if VL_PLATFORM(WINDOWS)

VL_DA_STRING wstring_to_string(const wchar_t* wstring) {
    int count = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, NULL, 0, NULL, NULL);
    VL_DA_STRING result = VL_DA_INIT_WITH_CAPACITY(char, count);
    WideCharToMultiByte(CP_UTF8, 0, wstring, -1, result, count, NULL, NULL);
    VL_DA_HEADER(result)->count = count;
    return result;
}

vl_result_t vl_font_search_query(VL_DA(vl_font_search_description_t)* results, const char *name) {
    if (!results) return VL_ERROR;
    if (!*results) {
        *results = VL_DA_INIT(vl_font_search_description_t);
    }
    const wchar_t* fontRegistryPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    
    HKEY roots[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };

    for (int i = 0; i < VL_ARR_LEN(roots); i++) {
        HKEY root = roots[i];
        HKEY hKey;
        if (RegOpenKeyExW(root, fontRegistryPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD value_count = 0;
            DWORD max_name_len = 0;
            DWORD max_value_len = 0;
            if (RegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &value_count, &max_name_len, &max_value_len, NULL, NULL) == ERROR_SUCCESS) {
                VL_DA(WCHAR) name_arr = VL_DA_INIT_WITH_CAPACITY(WCHAR, max_name_len + 1);
                VL_DA(BYTE) value_arr = VL_DA_INIT_WITH_CAPACITY(BYTE, max_value_len + 1);
                VL_DA_HEADER(name_arr)->count = max_name_len + 1;
                VL_DA_HEADER(value_arr)->count = max_value_len + 1;
                for (DWORD i = 0; i < value_count; ++i) {
                    DWORD name_size = max_name_len + 1;
                    DWORD value_size = max_value_len + 1;
                    DWORD type = 0;
                    if (RegEnumValueW(hKey, i, name_arr, &name_size, NULL, &type, value_arr, &value_size) == ERROR_SUCCESS) {
                        VL_DA_STRING reg_name = wstring_to_string(name_arr);
                        if (name && !vl_font_search_compare_family_names(reg_name, name)) {
                            VL_DA_FREE(reg_name);
                            continue;
                        }
                        *VL_DA_PUSH(*results, vl_font_search_description_t) = (vl_font_search_description_t) {
                            .name = reg_name,
                            .path = wstring_to_string((const WCHAR*) value_arr)
                        };
                    }
                }
                VL_DA_FREE(name_arr);
                VL_DA_FREE(value_arr);
            }
            RegCloseKey(hKey);
        }
    }

    return VL_SUCCESS;
}

#endif