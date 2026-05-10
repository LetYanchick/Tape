cmake_minimum_required(VERSION 3.16)
project(Tape)

add_executable(Tape
    main.cpp
    FileTape.cpp
    sorting.cpp
    config.cpp
)
target_include_directories(Tape PRIVATE ${CMAKE_SOURCE_DIR})

option(BUILD_TESTS "Build unit tests" ON)


if(BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(test_runner
        tests/test_FileTape.cpp
        tests/test_sorting.cpp
        FileTape.cpp
        sorting.cpp
        config.cpp
    )
    target_include_directories(test_runner PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/tests
    )
    target_link_libraries(test_runner GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(test_runner)
endif()

