/*
    managed_assert.h - provides VL_ASSERT macro
    with option to disable it with VELVET_NDEBUG macro
*/

#ifndef VELVET_MANAGED_ASSERT_H
#define VELVET_MANAGED_ASSERT_H

/*
    define VELVET_NDEBUG to disable assert specifically for velvet
    defining VELVET_NDEBUG doesn't disable asserts for other parts of code
    if velvet is integrated directly into your project
*/

// VELVET_NDEBUG_AT_ALL is defined if NDEBUG and VELVET_NDEBUG are defined at the same time
// this way we don't undefine NDEBUG by accident later
#if defined(VELET_NDEBUG) && defined(NDEBUG)
    #define VELVET_NDEBUG_AT_ALL
#endif

#if defined(VELVET_NDEBUG) && !defined(NDEBUG)
    #define NDEBUG
#endif

#include <assert.h>

#if defined(VELVET_NDEBUG) && !defined(VELVET_NDEBUG_AT_ALL)
    #undef NDEBUG
#endif

#define VL_ASSERT(CONDITION) \
    assert((CONDITION))

#endif // VELVET_MANAGED_ASSERT_H