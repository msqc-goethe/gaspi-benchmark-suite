cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

add_executable(gbs_check "gbs_check.c")
target_link_libraries(
  gbs_check PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_check PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_check PRIVATE c_std_11)
