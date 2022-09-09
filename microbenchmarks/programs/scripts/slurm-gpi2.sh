#!/bin/bash
#an example script for submiting a GPI-2 test (proc_init.bin) to
#a SLURM's queue using one process per node and enabling NUMA.

#SBATCH --partition=fuchs
#SBATCH --job-name gpi_test
#SBATCH --time=00:05:00
#SBATCH --nodes=16
#SBATCH --ntasks=16

module load gpi-2-1.5.1-gcc-4.8.5-2u3v7sj

gaspihome=$HOME/local
gaspirun=$gaspihome/bin/gaspi_run

$gaspirun -N /scratch/fuchs/aglippert/zhuz/gaspi/gaspi-benchmark/programs/gaspi-benchmark/build/gaspi-benchmark

