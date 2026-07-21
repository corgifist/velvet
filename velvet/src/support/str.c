#include "velvet/support/str.h"
#include <ctype.h>

// Source - https://stackoverflow.com/a/5820991
// Posted by Fred Foo, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-21, License - CC BY-SA 4.0

int vl_strcicmp(const char *a, const char *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

int vl_nstrcicmp(const char *a, const char *b, int length) {
    for (int i = 0; i < length; i++) {
        int d = tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
        if (d != 0) return d;
    }
    return 0;
}