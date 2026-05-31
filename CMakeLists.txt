cmake_minimum_required(VERSION 3.16)
project(velvet LANGUAGES C)

add_executable(velvet src/main.c include/velvet.h)
target_include_directories(velvet PRIVATE include)