#ifndef VELVET_SUPPORT_ALPHANUM_H
#define VELVET_SUPPORT_ALPHANUM_H

#include "velvet/common.h"
#include "velvet/support/api.h"
#include "velvet/vendor/utf8.h"

VL_API bool vl_isalphanum(UChar32 codepoint);
VL_API bool vl_isalphabetical(UChar32 codepoint);
VL_API bool vl_isnumeric(UChar32 codepoint);

#endif // VELVET_SUPPORT_ALPHANUM_H