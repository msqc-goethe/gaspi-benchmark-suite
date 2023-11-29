cmake_minimum_required(VERSION 3.5)

find_package(GPI2 REQUIRED)
find_package(Threads REQUIRED)

add_executable(gbs_info "gbs_info.c")
target_link_libraries(
  gbs_info PRIVATE "GPI2::GPI2"
)
target_include_directories(
  gbs_info PRIVATE "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_info PRIVATE c_std_11)

install(TARGETS gbs_info RUNTIME DESTINATION bin/gaspi-info)
