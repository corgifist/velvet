if (CMAKE_BUILD_TYPE MATCHES "Debug")
    option(USE_ASAN "Use AddressSanitizer (ON for debug builds)" ON)
else()
    option(USE_ASAN "Use AddressSanitizer (ON for debug builds)" OFF)
endif()

option(BUILD_UNIVERSAL_PLATFORM "Build universal platform" ON)
option(BUILD_DOM_TEXT_NODE "Include support for raw text HTML node" ON)
option(BUILD_DOM_BODY_NODE "Include support for <body> HTML node" ON)
option(BUILD_DOM_HTML_NODE "Include support for <html> HTML node" ON)

find_package(cglm CONFIG REQUIRED)
pkg_search_module(ICU REQUIRED IMPORTED_TARGET icu-io icu-uc)

add_subdirectory(include)
add_subdirectory(src)

add_library(velvet ${Velvet_Sources} ${Velvet_Headers})

target_include_directories(velvet PUBLIC include)
target_include_directories(velvet PRIVATE include/velvet/)
target_include_directories(velvet PRIVATE ${Velvet_Private_Include_Dirs})

target_link_libraries(velvet PUBLIC PkgConfig::ICU)
target_link_libraries(velvet PRIVATE cglm::cglm)

if(BUILD_UNIVERSAL_PLATFORM)
    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(velvet PRIVATE glfw)
endif()

target_compile_definitions(velvet PRIVATE VL_EXPORT_SYMBOLS)
target_compile_definitions(velvet PUBLIC VL_FEATURE_UNIVERSAL_PLATFORM=$<BOOL:${BUILD_UNIVERSAL_PLATFORM}>)
target_compile_definitions(velvet PUBLIC VL_FEATURE_DOM_TEXT_NODE=$<BOOL:${BUILD_DOM_TEXT_NODE}>)
target_compile_definitions(velvet PUBLIC VL_FEATURE_DOM_BODY_NODE=$<BOOL:${BUILD_DOM_BODY_NODE}>)
target_compile_definitions(velvet PUBLIC VL_FEATURE_DOM_HTML_NODE=$<BOOL:${BUILD_DOM_HTML_NODE}>)

# https://felsoci.sk/blog/using-address-sanitizer-asan-in-a-cmake-project.html
if(USE_ASAN)
  set(
    CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -fsanitize=undefined -fsanitize=address"
  )
  target_link_options(velvet
    BEFORE PUBLIC -fsanitize=undefined PUBLIC -fsanitize=address
  )
endif()

set_target_properties(velvet PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)