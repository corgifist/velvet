if(BUILD_UNIVERSAL_PLATFORM)
    add_subdirectory(universal)
endif()

set(Velvet_Sources
    ${Velvet_Sources}
    src/platform/context.c
    PARENT_SCOPE
)

set(Velvet_Private_Include_Dirs
    ${Velvet_Private_Include_Dirs}
    PARENT_SCOPE
)