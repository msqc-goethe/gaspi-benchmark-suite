cmake_minimum_required(VERSION 3.5)

find_package(GPI2 REQUIRED)
find_package(Threads REQUIRED)

function(settings target)
  target_link_libraries(
    ${target} PRIVATE "GPI2::GPI2" "Threads::Threads" "m"
  )
  target_include_directories(
    ${target} PRIVATE "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
  )
  target_compile_features(${target} PRIVATE c_std_11)
endfunction()

set(EXE "gbs_write_bw" "gbs_write_lat" "gbs_write_bibw" "gbs_read_bw"
        "gbs_read_lat"
)
foreach(APP IN LISTS EXE)
  add_executable(${APP} "${APP}.c" "../../util/util.c" "../../util/util_memory.c" "../../util/stopwatch.c")
  settings(${APP})
endforeach()

install(TARGETS ${EXE} RUNTIME DESTINATION bin/one-sided)
