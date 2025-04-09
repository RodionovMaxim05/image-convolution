file(GLOB_RECURSE C_SOURCES ./*.c)

add_executable(image-convolution ${C_SOURCES})

target_include_directories(image-convolution PRIVATE ${CMAKE_SOURCE_DIR}/stb_image)

target_link_libraries(image-convolution PRIVATE m)

target_compile_options(image-convolution PRIVATE -Wall -Wextra -Wpedantic -Werror)
