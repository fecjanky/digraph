cmake_minimum_required(VERSION 3.0.2)

enable_testing()

project("digraph")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(config)

add_library(Catch_lib INTERFACE)

file(GLOB_RECURSE catch_source_files  Catch/include/ *.(h|hpp))

target_sources(Catch_lib INTERFACE ${catch_source_files})

target_include_directories(Catch_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Catch/include)


file(GLOB digraph_source_files  include/*.h)

add_library(digraph_lib INTERFACE)

target_sources(digraph_lib INTERFACE ${digraph_source_files})

target_include_directories(digraph_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(digraph_lib INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>  
)

add_subdirectory(test)

