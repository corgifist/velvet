#include "velvet/support/alphanum.h"
#include "velvet/support/memory.h"

#include "alphanum.h"

bool vl_isalphanum(UChar32 codepoint) {
    return vl_isalphabetical(codepoint) || vl_isnumeric(codepoint);
}

bool vl_isalphabetical(UChar32 codepoint) {
    if (codepoint >= VL_ARR_LEN(s_alphanum)) return false;
    return s_alphanum[codepoint] == VL_ALPHANUM_ALPHA;
}

bool vl_isnumeric(UChar32 codepoint) {
    if (codepoint >= VL_ARR_LEN(s_alphanum)) return false;
    return s_alphanum[codepoint] == VL_ALPHANUM_NUM;
}