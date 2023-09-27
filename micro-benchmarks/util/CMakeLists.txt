cmake_minimum_required(VERSION 3.5)

find_package(GPI2 REQUIRED)

add_library(benchmark_utility "stopwatch.c" "util.c" "util_memory.c")
target_link_libraries(benchmark_utility PRIVATE "GPI2::GPI2" "m" "c")
target_compile_features(benchmark_utility PRIVATE c_std_11)
