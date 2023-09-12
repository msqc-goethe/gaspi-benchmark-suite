cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

function(settings target)
  target_link_libraries(
    ${target} PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
  )
  target_include_directories(
    ${target} PRIVATE "${GPI2_INCLUDE_DIRS}"
                      "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
  )
  target_compile_features(${target} PRIVATE c_std_11)
endfunction()

add_executable(gbs_write_notify_bw "gbs_write_notify_bw.c")
settings(gbs_write_notify_bw)
add_executable(gbs_write_notify_lat "gbs_write_notify_lat.c")
settings(gbs_write_notify_lat)
add_executable(gbs_read_notify_lat "gbs_read_notify_lat.c")
settings(gbs_read_notify_lat)
add_executable(gbs_read_notify_bw "gbs_read_notify_bw.c")
settings(gbs_read_notify_bw)
add_executable(gbs_write_notify_bibw "gbs_write_notify_bibw.c")
settings(gbs_write_notify_bibw)

install(TARGETS gbs_write_notify_bw gbs_write_notify_lat gbs_write_notify_bibw
                gbs_read_notify_bw gbs_read_notify_lat
        RUNTIME DESTINATION bin/one-sided-extended
)
