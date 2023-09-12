cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

add_executable(gbs_atomic_fadd "gbs_atomic_fadd.c")
target_link_libraries(
  gbs_atomic_fadd PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_atomic_fadd PRIVATE "${GPI2_INCLUDE_DIRS}"
                          "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_atomic_fadd PRIVATE c_std_11)

add_executable(gbs_atomic_cas "gbs_atomic_cas.c")
target_link_libraries(
  gbs_atomic_cas PRIVATE "benchmark_utility" "${GPI2_LIBRARIES}"
)
target_include_directories(
  gbs_atomic_cas PRIVATE "${GPI2_INCLUDE_DIRS}"
                         "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_atomic_cas PRIVATE c_std_11)

install(TARGETS gbs_atomic_fadd gbs_atomic_cas RUNTIME DESTINATION bin/atomic)
