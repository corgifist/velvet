add_library(velvet_support)

add_subdirectory(include)
add_subdirectory(src)

target_include_directories(velvet_support PUBLIC include)

if(ENABLE_ASAN)
    target_link_libraries(velvet_support INTERFACE velvet_sanitizers)
endif()