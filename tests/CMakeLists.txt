file(GLOB_RECURSE C_SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
list(REMOVE_ITEM C_SOURCES ${CMAKE_SOURCE_DIR}/src/main.c)

add_mocked_test(main
    SOURCES ${C_SOURCES}
    LINK_LIBRARIES m
    )

target_include_directories(test_main PRIVATE ${CMAKE_SOURCE_DIR}/stb_image)
