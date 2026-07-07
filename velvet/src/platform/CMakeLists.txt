if(BUILD_UNIVERSAL_PLATFORM)
    add_subdirectory(universal)
endif()

set(Velvet_Sources
    ${Velvet_Sources}
    PARENT_SCOPE
)
