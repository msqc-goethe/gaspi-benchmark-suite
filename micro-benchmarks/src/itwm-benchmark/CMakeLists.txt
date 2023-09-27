cmake_minimum_required(VERSION 3.5)

find_package(GPI2 REQUIRED)
find_package(Threads REQUIRED)

function(settings target)
  target_link_libraries(
    ${target} PRIVATE "benchmark_utility" "GPI2::GPI2" "Threads::Threads"
  )
  target_include_directories(${target} PRIVATE "include")
  target_compile_features(${target} PRIVATE c_std_11)
endfunction()

set(EXE
    "write_bw"
    "read_bw"
    "write_lat"
    "write_notify_lat"
    "write_notify_bw"
    "read_notify_bw"
    "ping_pong"
    "nb_allreduce"
    "nb_barrier"
    "init_time"
)

foreach(APP IN LISTS EXE)
  add_executable(${APP} "${APP}.c" "common.c")
  settings(${APP})
endforeach()
