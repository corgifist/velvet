set(CMAKE_BUILD_TYPE Debug)
add_executable(velvet_test main.c)
target_link_libraries(velvet_test PRIVATE velvet)