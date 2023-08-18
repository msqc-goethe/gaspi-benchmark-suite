cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

add_executable(gbs_barrier "gbs_barrier.c")
target_link_libraries(
  gbs_barrier PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_barrier PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_barrier PRIVATE c_std_11)

add_executable(gbs_allreduce "gbs_allreduce.c")
target_link_libraries(
  gbs_allreduce PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_allreduce PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_allreduce PRIVATE c_std_11)

install(TARGETS gbs_barrier gbs_allreduce RUNTIME DESTINATION bin/collective)
