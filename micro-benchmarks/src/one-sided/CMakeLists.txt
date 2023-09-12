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

add_executable(gbs_write_bw "gbs_write_bw.c")
settings(gbs_write_bw)
add_executable(gbs_write_lat "gbs_write_lat.c")
settings(gbs_write_lat)
add_executable(gbs_write_bibw "gbs_write_bibw.c")
settings(gbs_write_bibw)
add_executable(gbs_read_bw "gbs_read_bw.c")
settings(gbs_read_bw)
add_executable(gbs_read_lat "gbs_read_lat.c")
settings(gbs_read_lat)

install(TARGETS gbs_write_bw gbs_write_lat gbs_read_bw gbs_read_lat
                gbs_write_bibw RUNTIME DESTINATION bin/one-sided
)
