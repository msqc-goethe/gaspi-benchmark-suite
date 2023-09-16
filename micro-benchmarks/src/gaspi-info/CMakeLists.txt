cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

add_executable(gbs_info "gbs_info.c")
target_link_libraries(
  gbs_info PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_info PRIVATE "${GPI2_INCLUDE_DIRS}"
                         "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_info PRIVATE c_std_11)

install(TARGETS gbs_info RUNTIME DESTINATION bin/gaspi-info)
