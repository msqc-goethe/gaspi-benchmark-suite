cmake_minimum_required(VERSION 3.5)

find_package(GPI2 REQUIRED)
find_package(Threads REQUIRED)

function(settings target)
  target_link_libraries(
    ${target} PRIVATE "benchmark_utility" "GPI2::GPI2" "Threads::Threads"
  )
  target_include_directories(
    ${target} PRIVATE "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
  )
  target_compile_features(${target} PRIVATE c_std_11)
endfunction()

add_executable(gbs_notification_rate "gbs_notification_rate.c")
settings(gbs_notification_rate)

add_executable(gbs_notification_ping_pong "gbs_notification_ping_pong.c")
settings(gbs_notification_ping_pong)

install(TARGETS gbs_notification_rate gbs_notification_ping_pong
        RUNTIME DESTINATION bin/notification
)
