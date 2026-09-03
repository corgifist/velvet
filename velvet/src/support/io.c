#include "velvet/support/io.h"
#include "support/da.h"
#include "support/memory.h"
#include <stdio.h>

VL_DA(vl_byte_t) vl_io_read_file(const char *path) {
    if (!path) return NULL;
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    VL_DA(vl_byte_t) result = VL_DA_INIT_WITH_CAPACITY(vl_byte_t, size + 1);
    VL_DA_HEADER(result)->count = size + 1;
    fread(result, 1, size, f);
    result[size] = '\0';
    fclose(f);
    return result;
}