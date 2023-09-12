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

add_executable(gbs_barrier "gbs_barrier.c")
settings(gbs_barrier)
add_executable(gbs_allreduce "gbs_allreduce.c")
settings(gbs_allreduce)
install(TARGETS gbs_barrier gbs_allreduce RUNTIME DESTINATION bin/collective)
