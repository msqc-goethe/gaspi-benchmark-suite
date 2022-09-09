#Hier bitte die richtigen Pfade für GASPI und MPI/SHMEM angeben
#bestimmen wie viele Ranks ich bekomme, Achtung es läuft dann einmal für gaspi, mpi und shmem durch
#MFILE muss die maximale Anzahl von den bestimmten Ranks an Einträgen enthalten, zusätzlich müssen sie wie im Beispiel mfile sortiert sein
#zuletzt mit bash eval.sh die Evaluation ausführen

#!/bin/bash

# Do not touch this: Path of THIS file
HERE="$(pwd)"
MACHINE_FILE="machines"
GASPI_RUN_FILE="${HERE}/wrapper.sh"

# Modify this:
GPI_DIR="/home/fuchs/aglippert/sneuwirth/opt"
GPI_CONFIG="/home/fuchs/aglippert/sneuwirth/opt/lib64"
MPI_LD_FLAGS="-I/usr/include/openmpi3-x86_64/"
MPI_LIB_FLAGS="-L/usr/lib64/openmpi3/lib/ -lmpi"
ranks=(32 64)
#Adapt this line to switch between gaspi, mpi and shmem
GASPI_MPI_SHMEM="gaspi"
