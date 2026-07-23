# Cross-platform implementation of APIs like vl_window etc.
if(BUILD_UNIVERSAL_PLATFORM)
    add_subdirectory(universal)
endif()

set(Velvet_Headers
    ${Velvet_Headers}
    include/velvet/platform/context.h
    PARENT_SCOPE
)
