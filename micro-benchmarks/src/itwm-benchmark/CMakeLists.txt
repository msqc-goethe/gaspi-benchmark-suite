cmake_minimum_required(VERSION 3.2)

find_package(GPI2 REQUIRED)

add_executable(write_bw "common.c" "write_bw.c")
target_link_libraries(write_bw PRIVATE "${GPI2_LIBRARIES}")
target_include_directories(write_bw PRIVATE "${GPI2_INCLUDE_DIRS}" "include")
target_compile_features(write_bw PRIVATE c_std_11)

add_executable(read_bw "common.c" "read_bw.c")
target_link_libraries(read_bw PRIVATE "${GPI2_LIBRARIES}")
target_include_directories(read_bw PRIVATE "${GPI2_INCLUDE_DIRS}" "include")
target_compile_features(read_bw PRIVATE c_std_11)

add_executable(write_lat "common.c" "write_lat.c")
target_link_libraries(write_lat PRIVATE "${GPI2_LIBRARIES}")
target_include_directories(write_lat PRIVATE "${GPI2_INCLUDE_DIRS}" "include")
target_compile_features(write_lat PRIVATE c_std_11)

add_executable(write_notify_lat "common.c" "write_notify_lat.c")
target_link_libraries(write_notify_lat PRIVATE "${GPI2_LIBRARIES}")
target_include_directories(
  write_notify_lat PRIVATE "${GPI2_INCLUDE_DIRS}" "include"
)
target_compile_features(write_notify_lat PRIVATE c_std_11)

add_executable(write_notify_bw "common.c" "write_notify_bw.c")
target_link_libraries(write_notify_bw PRIVATE "${GPI2_LIBRARIES}")
target_include_directories(
  write_notify_bw PRIVATE "${GPI2_INCLUDE_DIRS}" "include"
)
target_compile_features(write_notify_bw PRIVATE c_std_11)

add_executable(read_notify_bw "common.c" "read_notify_bw.c")
target_link_libraries(read_notify_bw PRIVATE "${GPI2_LIBRARIES}")
target_include_directories(
  read_notify_bw PRIVATE "${GPI2_INCLUDE_DIRS}" "include"
)
target_compile_features(read_notify_bw PRIVATE c_std_11)

add_executable(ping_pong "common.c" "ping_pong.c")
target_link_libraries(ping_pong PRIVATE "${GPI2_LIBRARIES}")
target_include_directories(ping_pong PRIVATE "${GPI2_INCLUDE_DIRS}" "include")
target_compile_features(ping_pong PRIVATE c_std_11)

add_executable(nb_allreduce "common.c" "nb_allreduce.c")
target_link_libraries(nb_allreduce PRIVATE "${GPI2_LIBRARIES}")
target_include_directories(
  nb_allreduce PRIVATE "${GPI2_INCLUDE_DIRS}" "include"
)
target_compile_features(nb_allreduce PRIVATE c_std_11)

add_executable(nb_barrier "common.c" "nb_barrier.c")
target_link_libraries(nb_barrier PRIVATE "${GPI2_LIBRARIES}")
target_include_directories(nb_barrier PRIVATE "${GPI2_INCLUDE_DIRS}" "include")
target_compile_features(nb_barrier PRIVATE c_std_11)

add_executable(init_time "common.c" "init_time.c")
target_link_libraries(init_time PRIVATE "${GPI2_LIBRARIES}")
target_include_directories(init_time PRIVATE "${GPI2_INCLUDE_DIRS}" "include")
target_compile_features(init_time PRIVATE c_std_11)
