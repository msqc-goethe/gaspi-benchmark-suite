cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

add_executable(gbs_write_notify_bw "gbs_write_notify_bw.c")
target_link_libraries(
  gbs_write_notify_bw PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_write_notify_bw PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_write_notify_bw PRIVATE c_std_11)

add_executable(gbs_write_notify_lat "gbs_write_notify_lat.c")
target_link_libraries(
  gbs_write_notify_lat PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_write_notify_lat PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_write_notify_lat PRIVATE c_std_11)

add_executable(gbs_read_notify_lat "gbs_read_notify_lat.c")
target_link_libraries(
  gbs_read_notify_lat PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_read_notify_lat PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_read_notify_lat PRIVATE c_std_11)

add_executable(gbs_read_notify_bw "gbs_read_notify_bw.c")
target_link_libraries(
  gbs_read_notify_bw PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_read_notify_bw PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_read_notify_bw PRIVATE c_std_11)

install(TARGETS gbs_write_notify_bw gbs_write_notify_lat gbs_read_notify_bw gbs_read_notify_lat RUNTIME DESTINATION bin/one-sided-extended)
