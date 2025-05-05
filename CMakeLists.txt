cmake_minimum_required(VERSION 3.10)

project(image-convolution LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2") 

add_subdirectory(src)

find_package(CMocka CONFIG)
if (CMocka_FOUND)
    add_subdirectory(tests)
    enable_testing()
endif(CMocka_FOUND)
