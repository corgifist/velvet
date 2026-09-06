#include "velvet/web/font_storage.h"
#include "support/da.h"
#include "support/io.h"
#include "support/result.h"

vl_result_t vl_web_font_storage_init(vl_web_font_storage_t *storage) {
    if (!storage) return VL_ERROR;
    storage->records = VL_DA_INIT(vl_web_font_storage_record_t);
    return VL_SUCCESS;
}

vl_web_font_storage_record_t *vl_web_font_storage_query(vl_web_font_storage_t *storage, const char *path) {
    if (!storage || !path) return NULL;
    for (int i = 0; i < VL_DA_LENGTH(storage->records); i++) {
        vl_web_font_storage_record_t *record = storage->records + i;
        if (record->path && strcmp(record->path, path) == 0) {
            return record;
        }
    }
    vl_web_font_storage_record_t record = {0};
    record.path = VL_DA_INIT_FROM_STRING(path);
    record.data = vl_io_read_file(path);
    if (record.data) record.len = VL_DA_LENGTH(record.data);
    return VL_DA_APPEND(storage->records, record);
}

vl_result_t vl_web_font_storage_deinit(vl_web_font_storage_t *storage) {
    if (!storage) return VL_ERROR;
    for (int i = 0; i < VL_DA_LENGTH(storage->records); i++) {
        vl_web_font_storage_record_t *record = storage->records + i;
        VL_DA_FREE(record->data);
        VL_DA_FREE(record->path);
    }
    VL_DA_FREE(storage->records);
    return VL_SUCCESS;
}