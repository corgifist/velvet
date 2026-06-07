cmake_minimum_required(VERSION 3.19)
find_package(PkgConfig REQUIRED)

project(velvet LANGUAGES C)

add_subdirectory(velvet)
add_subdirectory(test)