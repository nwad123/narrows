cmake_minimum_required(VERSION 3.30)

# import gtest
include(FetchContent)
if (NOT googletest_POPULATED)
    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip)
    FetchContent_MakeAvailable(googletest)
endif()
include(GoogleTest)

# makes defining a test easier
function(add_narrows_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} gtest gtest_main ${LIBS})

    gtest_discover_tests(${TESTNAME})
endfunction()

# add the tests
add_narrows_test(bounded bounded.cpp)
