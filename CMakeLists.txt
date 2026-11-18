cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(velvet LANGUAGES C)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

add_library(velvet_sanitizers INTERFACE)
target_compile_options(velvet_sanitizers 
    INTERFACE
    -fsanitize=address
    -fno-omit-frame-pointer
)
target_link_options(velvet_sanitizers INTERFACE -fsanitize=address)

add_subdirectory(velvet)
add_subdirectory(app)