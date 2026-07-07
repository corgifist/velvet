/*
    feature.h - utilities for checking availability of feature at compile time
    provides: FEATURE(NAME) macro
*/

#ifndef VELVET_SUPPORT_FEATURE_H
#define VELVET_SUPPORT_FEATURE_H

#ifndef VL_FEATURE_UNIVERSAL_PLATFORM
#define VL_FEATURE_UNIVERSAL_PLATFORM 0
#endif // VELVET_FEATURE_UNIVERSAL_PLATFORM

#define VL_FEATURE(NAME) VL_FEATURE_ ## NAME

#endif // VELVET_SUPPORT_FEATURE_H