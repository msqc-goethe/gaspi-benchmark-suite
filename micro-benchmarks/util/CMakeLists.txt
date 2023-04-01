cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

add_library(benchmark_utility "stopwatch.c" "util.c" "util_memory.c")
target_link_libraries(benchmark_utility PRIVATE "${GPI2_LIBRARIES}" "m" "c")
target_include_directories(benchmark_utility PRIVATE "${GPI2_INCLUDE_DIRS}")
target_compile_features(benchmark_utility PRIVATE c_std_11)
