cmake_minimum_required(VERSION 3.10)

project(image-convolution LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(CMocka CONFIG)
if (CMocka_FOUND)
    include(AddCMockaTest)
    include(AddMockedTest)
    add_subdirectory(tests)
    enable_testing()
endif(CMocka_FOUND)
