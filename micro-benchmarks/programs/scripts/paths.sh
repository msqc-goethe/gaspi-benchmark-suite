#!/bin/bash

# Do not touch this: Path of THIS file
HERE="$(pwd)"

# Modify this:
GPI_DIR="/scratch/fuchs/aglippert/zhuz/tools/GPI-2"
MPI_PREFIX="/usr/lib64/openmpi/"
MPI_BIN="${MPI_PREFIX}/bin/mpirun"
INCLUDE_DIR="${HERE}/../include"
PROGRAM_DIR="${HERE}/../gaspi-benchmark"
BUILD_DIR="${PROGRAM_DIR}/build"
EXE_NAME="gaspi-benchmark"
GASPI_RUN_FILE="run-va.sh"
