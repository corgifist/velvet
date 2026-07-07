set(CMAKE_BUILD_TYPE Debug)
add_subdirectory(include)
add_subdirectory(src)

option(BUILD_UNIVERSAL_PLATFORM "Build universal platform" ON)

pkg_search_module(ICU REQUIRED IMPORTED_TARGET icu-io icu-uc)

add_library(velvet ${Velvet_Sources} ${Velvet_Headers})

target_include_directories(velvet PUBLIC include)
target_include_directories(velvet PRIVATE include/velvet/)

target_link_libraries(velvet PUBLIC PkgConfig::ICU)

if(BUILD_UNIVERSAL_PLATFORM)
    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(velvet PRIVATE glfw)
endif()

target_compile_definitions(velvet PRIVATE VL_EXPORT_SYMBOLS)
target_compile_definitions(velvet PUBLIC VL_FEATURE_UNIVERSAL_PLATFORM=$<BOOL:${BUILD_UNIVERSAL_PLATFORM}>)

set_target_properties(velvet PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)