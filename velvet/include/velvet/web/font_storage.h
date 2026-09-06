#ifndef VELVET_WEB_FONT_STORAGE_H
#define VELVET_WEB_FONT_STORAGE_H

#include "velvet/support/da.h"
#include "velvet/support/result.h"

struct vl_web_font_storage_record {
    VL_DA_STRING path;
    vl_byte_t *data;
    size_t len;
};
typedef struct vl_web_font_storage_record vl_web_font_storage_record_t;

#define VL_FONT_STORAGE_RECORD(KIND, DATA, LEN) \
    ((vl_web_font_storage_record_t) {.kind = (KIND), .data = (vl_byte_t*) (DATA), .len = (size_t) (LEN)})

struct vl_web_font_storage {
    VL_DA(vl_web_font_storage_record_t) records;
};
typedef struct vl_web_font_storage vl_web_font_storage_t;

VL_API vl_result_t vl_web_font_storage_init(vl_web_font_storage_t *storage);
VL_API vl_web_font_storage_record_t *vl_web_font_storage_query(vl_web_font_storage_t *storage, const char *path);
VL_API vl_result_t vl_web_font_storage_deinit(vl_web_font_storage_t *storage);

#endif // VELVET_WEB_FONT_STORAGE_H