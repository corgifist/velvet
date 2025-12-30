#pragma once

#ifndef VELVET_PREVIEW
#define VELVET_PREVIEW 1
#endif // VELVET_PREVIEW

#ifndef VELVET_PREVIEW_BASE_URL
#define VELVET_PREVIEW_BASE_URL "http://localhost:5173"
#endif // VELVET_PREVIEW_BASE_URL

#ifndef VELVET_BASE_URL

#if VELVET_PREVIEW
#define VELVET_BASE_URL VELVET_PREVIEW_BASE_URL
#else
#define VELVET_BASE_URL "velvet://"
#endif

#endif // VELVET_BASE_URL


#include <string>
#include <string_view>

namespace velvet {

std::string construct_url(std::string_view asset);

/*
    velvet::format_string(std::string_view format, ...)

    formats string with given C-style format using vsnprintf

    NOTICE:
    even though this function does it's job, you should consider swithcing to fmt library
    fmt provides a safe and modern interface for formatting strings

    check https://github.com/fmtlib/fmt for more information

*/
std::string format_string(std::string_view format, ...);

inline std::string url(std::string_view url) {
    return VELVET_BASE_URL + std::string(url);
}

} // namespace velvet