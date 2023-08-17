cmake_minimum_required(VERSION 3.2)

option(USE_MPI3 "Build with MPI3 Support" OFF)
option(USE_SHMEM "Build with SHMEM Support" OFF)
option(USE_GPI2 "Build with GPI-2 Support" ON)

add_executable(ssca1 "gen_scal_data.c" "gen_sim_matrix.c" "glibc_sort.c" "main.c" "pairwise_align.c" "parameters.c" "scan_backwards.c" "util.c")

if(USE_GPI2)
	find_package(GPI2 REQUIRED)
	target_link_libraries(ssca1 PRIVATE "${GPI2_LIBRARIES}" "m")
	target_include_directories(ssca1 PRIVATE "${GPI2_INCLUDE_DIRS}") 
	target_compile_definitions(ssca1 PRIVATE USE_GASPI)
	target_compile_features(ssca1 PRIVATE c_std_11)
endif()

if(USE_MPI3)
	find_package(MPI REQUIRED)
	target_link_libraries(ssca1 PRIVATE "MPI::MPI_C" "m")
	target_compile_definitions(ssca1 PRIVATE USE_MPI3)
	target_compile_features(ssca1 PRIVATE c_std_11)
endif()

if(USE_SHMEM)
	find_package(MPI REQUIRED)
	target_link_libraries(ssca1 PRIVATE "MPI::MPI_C" "m")
	target_compile_definitions(ssca1 PRIVATE USE_SHMEM)
	target_compile_features(ssca1 PRIVATE c_std_11)
endif()
