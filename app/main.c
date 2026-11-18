#include "velvet/support/std.h"
#include "velvet/support/malloc.h"

int main(int argc, const char *argv[]) {
    int *p = VL_MALLOC(sizeof(int));
    *p = 72;
    printf("*p = %i\n", *p);
    VL_FREE(p);
    return 0;
}