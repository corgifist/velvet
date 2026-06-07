add_subdirectory(include)
add_subdirectory(src)

pkg_search_module(ICU REQUIRED IMPORTED_TARGET icu-io icu-uc)

add_library(velvet ${Velvet_Sources} ${Velvet_Headers})

target_include_directories(velvet PUBLIC include)
target_include_directories(velvet PRIVATE include/velvet/)

target_link_libraries(velvet PUBLIC PkgConfig::ICU)

target_compile_definitions(velvet PRIVATE VELVET_EXPORT_SYMBOLS)
set_target_properties(velvet PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)