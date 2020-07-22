#
# Main CMakelists for the boilerplate project.
#
# It aims to be a template and a CMake reference, and as such is documented as much as possible.
# While everything might not fit in any project, it should give good defaults and avoid CMake antipatterns.
# If you disagree with some pieces of advice given here, please discuss it with me by opening a Github Issue !
#
# Project specific options :
#   - TREE_BUILD_TESTS (requires BUILD_TESTING set to ON)
# Other options might be available through the cmake scripts including (not exhaustive):
#   - ENABLE_WARNINGS_SETTINGS
#   - ENABLE_LTO
#
cmake_minimum_required(VERSION 3.14)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

# Put the project early since modules might need to detect the compiler.
# More information https://cmake.org/cmake/help/latest/command/project.html
project(
    "cpp_trees"
    VERSION 0.1.0
    LANGUAGES C CXX
)
SET(CMAKE_CXX_STANDARD 17)
############################
##  Modules and scripts   ##
############################

# Standard CMake modules
#include(CTest)

# Custom modules and scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake") # Make our cmake scripts available


###############
##  OPTIONS  ##
###############

# External dependencies
add_subdirectory(external EXCLUDE_FROM_ALL)

# It is always easier to navigate in an IDE when projects are organized in folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#tests
set(BUILD_TESTS ON)


###############
##  Project  ##
###############

#=================#
#  btree library  #
#=================#

# Based on the https://github.com/vector-of-bool/pitchfork way of doing things
add_library(btree
    source/btree.cpp
    source/btree.h
)

target_link_libraries(btree
        PRIVATE
        fmt
        spdlog)


# Give a 'namespaced' name to libraries targets, as it can't be mistaken with system libraries
add_library(trees::btree ALIAS btree)


#============================#
# tree_benchmark executable  #
#============================#

# Always list the source files explicitly, including headers so that they are listed in the IDE
# If you need to use files based on a variable value, use target_sources
add_executable(tree_benchmark source/main.cpp)

target_link_libraries(tree_benchmark
        PUBLIC
        btree
        general
        fmt
        spdlog
        )


#============================#
#           Tests            #
#============================#

if(BUILD_TESTS)
    # Let the user add options to the test runner if needed
    set(TEST_RUNNER_PARAMS "--force-colors=true" CACHE STRING "Options to add to our test runners commands")
    # In a real project you most likely want to exclude test folders
    # list(APPEND CUSTOM_COVERAGE_EXCLUDE "/test/")
    add_subdirectory(tests)
    # You can setup some custom variables and add them to the CTestCustom.cmake.in template to have custom ctest settings
    # For example, you can exclude some directories from the coverage reports such as third-parties and tests
    configure_file(
            ${CMAKE_CURRENT_LIST_DIR}/cmake/CTestCustom.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake
            @ONLY
    )
endif()

