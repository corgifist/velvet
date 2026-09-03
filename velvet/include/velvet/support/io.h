#ifndef VELVET_SUPPORT_IO_H
#define VELVET_SUPPORT_IO_H

#include "velvet/support/api.h"
#include "velvet/support/result.h"
#include "velvet/support/da.h"

VL_API VL_DA(vl_byte_t) vl_io_read_file(const char *path);

#endif // VELVET_SUPPORT_IO_H