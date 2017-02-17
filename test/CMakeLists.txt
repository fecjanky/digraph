cmake_minimum_required(VERSION 3.0.2)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
include(config)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_STANDARD 14)

enable_testing()

project(digraph_test CXX)

file(GLOB test_source_files  *.cpp)

add_executable(digraph_test ${test_source_files})

#target_include_directories(digraph_test INTERFACE ${digraph_lib_INCLUDE_DIRS} ${Catch_lib_INCLUDE_DIRS})

target_link_libraries(digraph_test  digraph_lib  Catch_lib)

set_property(TARGET digraph_test PROPERTY CXX_STANDARD 14)

add_test(NAME test_digraph COMMAND digraph_test WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
