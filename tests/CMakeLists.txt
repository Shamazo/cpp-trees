cmake_minimum_required(VERSION 3.6)
# Note : must be included by master CMakeLists.txt
# Tests in static libraries might not get registered, see https://github.com/onqtam/doctest/blob/master/doc/markdown/faq.md#why-are-my-tests-in-a-static-library-not-getting-registered
# For this reason, and because it is interesting to have individual
# test executables for each library, it is suggested not to put tests directly in the libraries (even though doctest advocates this usage)
# Creating multiple executables is of course not mandatory, and one could use the same executable with various command lines to filter what tests to run.

add_executable(btree_tests btreetest.cpp ${CMAKE_SOURCE_DIR}/source/btree.h)
target_include_directories(btree_tests PRIVATE ${CMAKE_SOURCE_DIR}/source)
target_link_libraries(btree_tests doctest trees::btree spdlog)

add_test(
    NAME btree_tests
    COMMAND successtest ${TEST_RUNNER_PARAMS}
)
