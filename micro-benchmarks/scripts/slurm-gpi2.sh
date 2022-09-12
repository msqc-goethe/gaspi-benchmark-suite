#!/bin/bash
#an example script for submiting a GPI-2 test (proc_init.bin) to
#a SLURM's queue using one process per node and enabling NUMA.

#SBATCH --partition=fuchs
#SBATCH --job-name gpi_test
#SBATCH --time=00:05:00
#SBATCH --nodes=2
#SBATCH --ntasks=2

module load gpi-2-1.5.1-gcc-4.8.5-2u3v7sj

gaspihome=$HOME/local
gaspirun=$gaspihome/bin/gaspi_run
autokit=/home/fuchs/aglippert/zhuz/Author-Kit
builddir=/scratch/fuchs/aglippert/zhuz/gaspi/micro-benchmarks/programs/gaspi-benchmark/build

$autokit/collect_environment.sh
$gaspirun -N $builddir/gaspi-benchmark

