cmake_minimum_required(VERSION 3.21)
find_package(PkgConfig REQUIRED)

if(DEFINED VELVET_TOOLCHAIN_FILE)
    include("${VELVET_TOOLCHAIN_FILE}")
endif()

project(velvet LANGUAGES C CXX)

add_subdirectory(velvet)
add_subdirectory(test)