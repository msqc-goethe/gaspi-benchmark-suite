cmake_minimum_required(VERSION 3.5)
project(GASPI_BENCHMARK_SUITE)

option(BUILD_UBENCH "Build Micro Benchmarks" ON)
option(BUILD_APPS "Build Application Benchmarks" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

if(BUILD_UBENCH)
  add_subdirectory(micro-benchmarks)
  message(STATUS "Building Micro Benchmarks")
endif()

if(BUILD_APPS)
  add_subdirectory(application-benchmarks)
  message(STATUS "Building Application Benchmarks")
endif()
