#include "velvet/font/search.h"
#include "velvet/support/platform.h"

#if VL_PLATFORM(MAC)
#include "search/search_mac.c"
#endif // VL_PLATFORM(MAC)