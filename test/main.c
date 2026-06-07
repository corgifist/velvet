#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unicode/umachine.h>
#include <unistd.h>

#include "velvet/support/platform.h"
#include "velvet/support/da.h"

#include "velvet/html/lexer.h"

#define _USE_MATH_DEFINES
#include <math.h>

void da_test() {
    VL_DA(int) array = VL_DA_INIT(int);
    vl_da_header_t *header = VL_DA_HEADER_PTR(array);
    printf("%zu %zu %zu\n", header->capacity, header->count, header->element_size);
    VL_DA_FREE(array);

    VL_DA(int) array2 = VL_DA_INIT(int);
    for (int i = 0; i < 12; ++i) {
        *VL_DA_PUSH(array2, int) = (i + 1) * 13;
    }
    header = VL_DA_HEADER_PTR(array2);
    printf("%zu %zu %zu\n", header->capacity, header->count, header->element_size);
    for (int i = 0; i < header->count; ++i) {
        printf("%i: %i\n", i, array2[i]);
    }
    VL_DA_DELETE(array2, 0);
    VL_DA_DELETE(array2, 1);
    VL_DA_DELETE(array2, 1);
    VL_DA_DELETE(array2, 1);
    VL_DA_DELETE(array2, 1);
    printf("%zu %zu %zu\n", header->capacity, header->count, header->element_size);
    for (int i = 0; i < header->count; ++i) {
        printf("%i: %i\n", i, array2[i]);
    }
    VL_DA_FREE(array2);
}

void da_stress_test() {
    VL_DA(int) da = VL_DA_INIT(int);
    srand(time(NULL));
    for (int i = 0; i < 100; ++i) {
        *VL_DA_PUSH(da, int) = rand();
    }
    printf("the numbers are:\n");
    for (int i = 0; i < 100; i++) {
        printf("%i\n", da[i]);
    }
    printf("added 100 random numbers: %zu %zu %zu\n", VL_DA_HEADER_PTR(da)->capacity, VL_DA_HEADER_PTR(da)->count, VL_DA_HEADER_PTR(da)->element_size);
    for (int i = 0; i < 99; ++i) {
        VL_DA_DELETE(da, 0);
    }
    printf("deleted all numbers except one: %i\n", da[0]);
}

#include <unicode/utf8.h>

void icu_test() {
    const char *msg = u8"Hello, мир!";
    int i = 0;
    UChar32 c;
    size_t count = 0;
    do {
        U8_NEXT(msg, i, 0, c);
        if (c <= 0) break;
        count++;
        printf("%zu: %i (%s)\n", count, c, msg);
    } while (c > 0);
}

void lexer_test() {
    vl_html_lexer_t lexer = {0};
    const char *text = "Hello, мир!";
    if (vl_html_lexer_init(&lexer, text)) {
        printf("vl_html_lexer_init error\n");
        return;
    }
    printf("expect squiggles: %s\n", lexer.text);
    printf("length: %zu\n", lexer.length);
    if (vl_html_lexer_deinit(&lexer)) {
        printf("failed to deinit lexer\n");
        return;
    }
    printf("deinitialized lexer\n");
}

int main(int argc, const char *argv[]) {
    char cwd[256];
    getcwd(cwd, 256);
    printf("%s | %s\n", argv[0], cwd);
    printf("%0.2f\n", M_PI);

#if VL_PLATFORM(WINDOWS)
    printf("win32\n");
#elif VL_PLATFORM(GNU_LINUX)
    printf("gnu/linux\n");
#elif VL_PLATFORM(MAC)
    printf("mac\n");
#endif

    // da_stress_test();
    // icu_test();
    lexer_test();

    return 0;
}