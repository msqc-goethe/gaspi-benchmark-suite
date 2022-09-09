#!/bin/bash

# Do not touch this: Path of THIS file
HERE="$(pwd)"

# Modify this:
#GPI_DIR="/opt/gaspi/diana/gaspi_benchmark/GPI-2/"
GPI_DIR="/opt/gaspi/diana/gaspi_benchmark/GPI-2_Ethernet/"
GPI_CONFIG="/lib/lib64"
MPI_PREFIX="/usr/lib64/openmpi/"
MPI_BIN="${MPI_PREFIX}/bin/mpirun"
INCLUDE_DIR="${HERE}/../../include"
PROGRAM_DIR="/../gaspi_mytest/mytest/"
BUILD_DIR="${PROGRAM_DIR}/build"
EXE_NAME="use_different_queues_read_notify"
#EXE_NAME="ALL"
MACHINE_FILE="${HERE}/../script/mfile_eno1_1GB.txt"
GASPI_RUN_FILE="${HERE}/../script/wrapper.sh"
