#include "velvet/support/std.h"
#include "velvet/support/malloc.h"
#include "velvet/support/variadic.h"

#define VL_SUM3(A, B, C) (A + B + C)
#define VL_SUM2(A, B) (A + B)
#define VL_SUM1(A) (A)
#define VL_SUM0() 0
#define VL_SUM(...) VL_VA_DISPATCH(VL_SUM, __VA_ARGS__)

int main(int argc, const char *argv[]) {
    int *p = VL_MALLOC(sizeof(int));
    *p = 72;
    printf("*p = %i\n", *p);
    VL_FREE(p);

    printf("a, b, c: %i\n", VL_VA_NARGS(a, c,    c, ));
    printf(" : %i\n", VL_SUM());
    printf("5: %i\n", VL_SUM(5, ));
    printf("5 + 6: %i\n", VL_SUM(5, 6, ));
    printf("5 + 6 + 7: %i\n", VL_SUM(5, 6, 8, , , ,));

    return 0;
}