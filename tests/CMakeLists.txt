file(GLOB_RECURSE SRC_SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
list(REMOVE_ITEM SRC_SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/queue_mode/queue.c
    ${CMAKE_SOURCE_DIR}/src/queue_mode/threads.c
    ${CMAKE_SOURCE_DIR}/src/queue_mode/queue_dispatch.c
)

add_executable(unit_tests unit_tests.c utils_for_tests.c ${SRC_SOURCES})
add_executable(sequential_tests sequential_tests.c utils_for_tests.c ${SRC_SOURCES})
add_executable(parallel_tests parallel_tests.c utils_for_tests.c ${SRC_SOURCES})

target_link_libraries(unit_tests PRIVATE m cmocka)
target_link_libraries(sequential_tests PRIVATE m cmocka)
target_link_libraries(parallel_tests PRIVATE m cmocka)

add_test(NAME unit_tests COMMAND unit_tests)
add_test(NAME sequential_tests COMMAND sequential_tests)
add_test(NAME parallel_tests COMMAND parallel_tests)
