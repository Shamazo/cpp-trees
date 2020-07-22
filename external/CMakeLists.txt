project(external C CXX)

set(BUILD_TESTING_BCKP ${BUILD_TESTING})
set(BUILD_TESTING OFF CACHE BOOL "Force disable of tests for external dependencies" FORCE)

#Use fmtlib for formatting
add_subdirectory(fmt EXCLUDE_FROM_ALL)

#Use spdlog for logging, and tell it to use our version of fmtlib
set(SPDLOG_BUILD_SHARED ON)
set(DSPDLOG_BUILD_TESTS OFF)
add_subdirectory(spdlog EXCLUDE_FROM_ALL)
#target_compile_definitions(spdlog INTERFACE SPDLOG_FMT_EXTERNAL)

#Add new external (non-interface) targets to this list
set_target_properties(fmt PROPERTIES FOLDER "external")


#Doctest for unit tests
add_library(doctest INTERFACE)
target_include_directories(
    doctest
    INTERFACE
    doctest/doctest # note : will expose the parts/ folder...
)
target_compile_features(doctest INTERFACE cxx_std_11)


set(BUILD_TESTING ${BUILD_TESTING_BCKP} CACHE BOOL "Build tests (default variable for CTest)" FORCE) #Set it back to its past value
