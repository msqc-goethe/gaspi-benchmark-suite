cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

add_executable(gbs_notification_rate "gbs_notification_rate.c")
target_link_libraries(
  gbs_notification_rate PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_notification_rate PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_notification_rate PRIVATE c_std_11)

add_executable(gbs_notification_ping_pong "gbs_notification_ping_pong.c")
target_link_libraries(
  gbs_notification_ping_pong PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_notification_ping_pong PRIVATE "${GPI2_INCLUDE_DIRS}"
                       "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_notification_ping_pong PRIVATE c_std_11)
