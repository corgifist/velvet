add_library(velvet INTERFACE)

add_subdirectory(support)

target_link_libraries(velvet INTERFACE velvet_support)

set_target_properties(velvet PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

if(ENABLE_ASAN)
    target_link_libraries(velvet PRIVATE velvet_sanitizers)
endif()