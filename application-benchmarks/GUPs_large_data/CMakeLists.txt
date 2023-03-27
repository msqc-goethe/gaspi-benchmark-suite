cmake_minimum_required(VERSION 3.2)

option(USE_MPI3 "Build with MPI3 Support" OFF)
option(USE_SHMEM "Build with SHMEM Support" OFF)
option(USE_GPI2 "Build with MPI3 Support" ON)

add_executable(GUPs_large_data "SHMEMRandomAccess.c")

if(USE_GPI2)
	find_package(GPI2 REQUIRED)
	target_link_libraries(GUPs_large_data PRIVATE "${GPI2_LIBRARIES}")
	target_include_directories(GUPs_large_data PRIVATE "${GPI2_INCLUDE_DIRS}")
	target_compile_definitions(GUPs_large_data PRIVATE USE_GASPI)
	target_compile_features(GUPs_large_data PRIVATE c_std_11)
endif()

if(USE_MPI3)
	find_package(MPI REQUIRED)
	target_link_libraries(GUPs_large_data PRIVATE MPI::MPI_C)
	target_compile_definitions(GUPs_large_data PRIVATE USE_MPI3)
	target_compile_features(GUPs_large_data PRIVATE c_std_11)
endif()

if(USE_SHMEM)
	find_package(MPI REQUIRED)
	target_link_libraries(GUPs_large_data PRIVATE MPI::MPI_C)
	target_compile_definitions(GUPs_large_data PRIVATE USE_SHMEM)
	target_compile_features(GUPs_large_data PRIVATE c_std_11)
endif()
