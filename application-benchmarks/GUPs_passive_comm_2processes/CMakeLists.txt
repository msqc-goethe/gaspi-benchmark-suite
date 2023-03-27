cmake_minimum_required(VERSION 3.2)

option(USE_MPI3 "Build with MPI3 Support" OFF)
option(USE_SHMEM "Build with SHMEM Support" OFF)
option(USE_GPI2 "Build with MPI3 Support" ON)

add_executable(GUPs_passive_comm_2processes "GASPIRandomAccess.c")

if(USE_GPI2)
	find_package(GPI2 REQUIRED)
	target_link_libraries(GUPs_passive_comm_2processes PRIVATE "${GPI2_LIBRARIES}")
	target_include_directories(GUPs_passive_comm_2processes PRIVATE "${GPI2_INCLUDE_DIRS}")
	target_compile_definitions(GUPs_passive_comm_2processes PRIVATE USE_GASPI)
	target_compile_features(GUPs_passive_comm_2processes PRIVATE c_std_11)
endif()

if(USE_MPI3)
	find_package(MPI REQUIRED)
	target_link_libraries(GUPs_passive_comm_2processes PRIVATE MPI::MPI_C)
	target_compile_definitions(GUPs_passive_comm_2processes PRIVATE USE_MPI3)
	target_compile_features(GUPs_passive_comm_2processes PRIVATE c_std_11)
endif()

if(USE_SHMEM)
	find_package(MPI REQUIRED)
	target_link_libraries(GUPs_passive_comm_2processes PRIVATE MPI::MPI_C)
	target_compile_definitions(GUPs_passive_comm_2processes PRIVATE USE_SHMEM)
	target_compile_features(GUPs_passive_comm_2processes PRIVATE c_std_11)
endif()
