cmake_minimum_required(VERSION 3.20)

set(TESTS_TARGET okn-editor_tests)

find_package(doctest CONFIG QUIET)
if(NOT doctest_FOUND)
    find_package(doctest QUIET)
endif()

if(doctest_FOUND)
    file(GLOB_RECURSE ${TESTS_TARGET}_SRC
            CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

    if(${TESTS_TARGET}_SRC)
        add_executable(${TESTS_TARGET} ${${TESTS_TARGET}_SRC})
        target_link_libraries(${TESTS_TARGET} PRIVATE okn-editor doctest::doctest)
        add_test(NAME ${TESTS_TARGET} COMMAND ${TESTS_TARGET})
    endif()
endif()
