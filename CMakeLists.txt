cmake_minimum_required(VERSION 3.30)

# This is my cross-thread channel project
project(Narrows VERSION 0.1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add all the compiler warnings
include(cmake/CompilerWarnings.cmake)

# Set the type as debug for now
set(CMAKE_BUILD_TYPE Debug)

# Create compile commands for clangd to look for
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Create an interface library since this is header-only
add_library(Narrows INTERFACE)
target_include_directories(Narrows
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
narrows_set_target_warnings(Narrows)

# Specify include directories for the library
include_directories(include)

# Set up testing environment
enable_testing()
add_subdirectory(tests)
