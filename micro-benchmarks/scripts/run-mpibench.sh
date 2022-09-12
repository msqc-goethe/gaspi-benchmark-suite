#!/bin/bash

source paths.sh

mpiprefix="${MPI_PREFIX}/"
mpibin="${MPI_BIN}"
mfile="${MACHINE_FILE}"
osudir="${MPI_OMB_DIR}/mpi/"
imbdir="${MPI_IMB_DIR}/src_c/"
resultdir=$1
sizestop=$2
interface=$3
interfacename=$4

rmadir="$osudir/one-sided"
#colldir="$osudir/collective"


rmabench=(osu_get_bw osu_put_bw osu_put_bibw)
imbbench=(IMB-RMA)

if [ -z $interface ] ; then
	mpiparams="--allow-run-as-root --prefix $mpiprefix --machinefile $mfile -n 2 -N 1"
	interfacename="ib"
else
	mpiparams="--allow-run-as-root --prefix $mpiprefix --mca btl_tcp_if_include $interface --mca pml ob1 --mca btl tcp,self --machinefile $mfile -n 2 -N 1"
fi

if ! [ -d $resultdir ] ; then
	mkdir -p $resultdir
fi

for bench in ${rmabench[@]} ; do
	$mpibin $mpiparams $rmadir/$bench -m $sizestop | tee "${resultdir}/osu-${interfacename}-${bench}.txt"
done

# Run IMB

for bench in ${imbbench[@]} ; do
        $mpibin $mpiparams $imbdir/$bench | tee "${resultdir}/imb-${interfacename}-${bench}.txt"
done
