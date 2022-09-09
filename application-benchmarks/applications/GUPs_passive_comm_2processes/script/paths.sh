#!/bin/bash

# Do not touch this: Path of THIS file
HERE="$(pwd)"
MACHINE_FILE="${HERE}/mfile_ib0.txt"
GASPI_RUN_FILE="${HERE}/wrapper.sh"

# Modify this:
GPI_DIR="/opt/gaspi/diana/gaspi_benchmark/GPI-2_Infiniband/"
GPI_CONFIG="/lib/lib64"
MPI_LD_FLAGS="-I/usr/include/mpich-3.2-x86_64/ -I/usr/local/include/" 
MPI_LIB_FLAGS="-L/usr/lib64/mpich-3.2/lib -L/usr/local/lib -lmpi"
#Adapt this line to switch between gaspi, mpi and shmem
GASPI_MPI_SHMEM="gaspi"
