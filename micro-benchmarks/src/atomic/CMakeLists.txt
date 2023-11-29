cmake_minimum_required(VERSION 3.5)

find_package(GPI2 REQUIRED)
find_package(Threads REQUIRED)

add_executable(gbs_atomic_fadd "gbs_atomic_fadd.c" "../../util/util.c" "../../util/util_memory.c" "../../util/stopwatch.c")
target_link_libraries(
  gbs_atomic_fadd PRIVATE "GPI2::GPI2" "Threads::Threads" "m"
)
target_include_directories(
  gbs_atomic_fadd PRIVATE "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_atomic_fadd PRIVATE c_std_11)

add_executable(gbs_atomic_cas "gbs_atomic_cas.c" "../../util/util.c" "../../util/util_memory.c" "../../util/stopwatch.c")
target_link_libraries(
  gbs_atomic_cas PRIVATE "GPI2::GPI2" "Threads::Threads" "m"
)
target_include_directories(
  gbs_atomic_cas PRIVATE "${PROJECT_SOURCE_DIR}/micro-benchmarks/util"
)
target_compile_features(gbs_atomic_cas PRIVATE c_std_11)

install(TARGETS gbs_atomic_fadd gbs_atomic_cas RUNTIME DESTINATION bin/atomic)
