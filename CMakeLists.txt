cmake_minimum_required(VERSION 3.14)
project(image_processor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(image_processor
        image_processor.cpp
        bmp_structures.cpp
        filters.cpp
)

target_compile_options(image_processor PRIVATE -O3 -march=native)

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    include(CTest)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(test_image_processor
            test_image_processor.cpp
            bmp_structures.cpp
            filters.cpp
    )

    target_link_libraries(test_image_processor
            gtest_main
            gtest
            ${CMAKE_THREAD_LIBS_INIT}
    )

    target_compile_options(test_image_processor PRIVATE -O0 -g)

    add_test(NAME ImageProcessorTest COMMAND test_image_processor)
endif()