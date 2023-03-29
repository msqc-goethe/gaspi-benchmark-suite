cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

add_executable(gbs_write_bw "gbs_write_bw.c")
target_link_libraries(
  gbs_write_bw PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_write_bw PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_write_bw PRIVATE c_std_11)

add_executable(gbs_write_lat "gbs_write_lat.c")
target_link_libraries(
  gbs_write_lat PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_write_lat PRIVATE "${GPI2_INCLUDE_DIRS}"
                            "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_write_lat PRIVATE c_std_11)

add_executable(gbs_write_bibw "gbs_write_bibw.c")
target_link_libraries(
  gbs_write_bibw PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_write_bibw PRIVATE "${GPI2_INCLUDE_DIRS}"
                            "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_write_bibw PRIVATE c_std_11)

add_executable(gbs_read_bw "gbs_read_bw.c")
target_link_libraries(
  gbs_read_bw PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_read_bw PRIVATE "${GPI2_INCLUDE_DIRS}"
                            "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_read_bw PRIVATE c_std_11)

add_executable(gbs_read_lat "gbs_read_lat.c")
target_link_libraries(
  gbs_read_lat PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_read_lat PRIVATE "${GPI2_INCLUDE_DIRS}"
                            "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_read_lat PRIVATE c_std_11)
