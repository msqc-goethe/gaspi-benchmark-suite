cmake_minimum_required(VERSION 3.5)

find_package(GPI2 REQUIRED)
find_package(Threads REQUIRED)

function(settings target)
  target_link_libraries(${target} PRIVATE "benchmark_utility" "GPI2::GPI2")
  target_include_directories(
    ${target} PRIVATE "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
  )
  target_compile_features(${target} PRIVATE c_std_11)
endfunction()

add_executable(gbs_passive_bw "gbs_passive_bw.c")
settings(gbs_passive_bw)
add_executable(gbs_passive_lat "gbs_passive_lat.c")
settings(gbs_passive_lat)

install(TARGETS gbs_passive_bw gbs_passive_lat RUNTIME DESTINATION bin/passive)
