cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

add_executable(gbs_passive_bw "gbs_passive_bw.c")
target_link_libraries(
  gbs_passive_bw PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_passive_bw PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_passive_bw PRIVATE c_std_11)

add_executable(gbs_passive_lat "gbs_passive_lat.c")
target_link_libraries(
  gbs_passive_lat PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_passive_lat PRIVATE "${GPI2_INCLUDE_DIRS}"
                            "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_passive_lat PRIVATE c_std_11)

install(TARGETS gbs_passive_bw gbs_passive_lat RUNTIME DESTINATION bin/passive)
