cmake_minimum_required(VERSION 3.2)

option(USE_MPI3 "Build with MPI3 Support" OFF)
option(USE_SHMEM "Build with SHMEM Support" OFF)
option(USE_GPI2 "Build with GPI-2 Support" ON)

if(USE_GPI2)
	add_subdirectory(gaspi)
endif()

if(USE_MPI3)
	add_subdirectory(mpi)
endif()

if(USE_SHMEM)
	add_subdirectory(shmem)
endif()
