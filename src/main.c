#include <stdio.h>
#include <unistd.h>

#include "memory.h"
#include "velvet.h"
#include "da.h"
#include "platform.h"

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
    VL_DA_DELETE_WITH_REALLOCATOR(array2, 0, VL_REALLOC);
    VL_DA_DELETE_WITH_REALLOCATOR(array2, 1, VL_REALLOC);
    VL_DA_DELETE_WITH_REALLOCATOR(array2, 1, VL_REALLOC);
    VL_DA_DELETE_WITH_REALLOCATOR(array2, 1, VL_REALLOC);
    VL_DA_DELETE_WITH_REALLOCATOR(array2, 1, VL_REALLOC);
    printf("%zu %zu %zu\n", header->capacity, header->count, header->element_size);
    for (int i = 0; i < header->count; ++i) {
        printf("%i: %i\n", i, array2[i]);
    }
    VL_DA_FREE(array2);
}

int main(int argc, const char *argv[]) {
    char cwd[256];
    getcwd(cwd, 256);
    printf("%s | %s\n", argv[0], cwd);
    printf("%0.2f\n", M_PI);

#if VL_PLATFORM(WINDOWS)
    printf("win32\n");
#elif VL_PLATFORM(GNU_LINUX)
    printf("gnu linux\n");
#elif VL_PLATFORM(MAC)
    printf("mac\n");
#endif

    da_test();

    void *mem = VL_MALLOC(sizeof(int));
    printf("void: %i\n", *((int*) mem));
    VL_FREE(mem);

    return 0;
}