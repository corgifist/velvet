#ifndef VELVET_ASSERT_H
#define VELVET_ASSERT_H

#ifdef VELVET_NDEBUG
#define NDEBUG
#endif

#include <assert.h>

#define VL_ASSERT(CONDITION) \
    assert((CONDITION))

#endif // VELVET_ASSERT_H