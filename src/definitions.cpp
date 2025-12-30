#include "definitions.h"
#include <cstdarg>
#include <cstdio>

namespace velvet {

std::string construct_url(std::string_view t_asset) {
    std::string url = VELVET_PREVIEW_BASE_URL + std::string(t_asset[0] == '/' ? "" : "/") + std::string(t_asset);
    return url;
}

std::string format_string(std::string_view format, ...) {
    va_list list, list2;
    va_start(list, format);
    va_copy(list2, list);
    size_t length = std::vsnprintf(nullptr, 0, format.data(), list);
    va_end(list);
    std::string result(' ', length);
    std::vsnprintf(result.data(), result.length(), format.data(), list2);
    return result;
}

} // namespace velvet