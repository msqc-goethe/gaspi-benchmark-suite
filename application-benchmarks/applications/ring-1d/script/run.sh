source paths.sh

exePath="$GPI_DIR/bin/gaspi_run"
machineFile="$MACHINE_FILE"
runfile="wrapper.sh"
gpi_dir=$GPI_DIR
gpi_config=$GPI_CONFIG
buildDir="${HERE}/../build"
exe_name="ring_gaspi"
rfile=$buildDir/$exe_name
chmod +rx $runfile
gaspi_mpi_shmem=$GASPI_MPI_SHMEM
ranks="4"
chmod +rx $runfile
#exe_path is the gaspi_run command
#machine file defines the running nodes
#runfile is the wrapper file, which executes the compiled file
#all the rest: arguments to the wrapper file, pathes to the outputfile,...
if [ $gaspi_mpi_shmem = "gaspi" ] ; then
    $exePath -m $machineFile  -n $ranks  $runfile $gpi_dir$gpi_config $rfile # > test.txt
fi
if [ $gaspi_mpi_shmem = "mpi" ] ; then
    mpirun -np $ranks -f $machineFile ./../build/ring_mpi 
fi
if [ $gaspi_mpi_shmem = "shmem" ] ; then
    mpirun -np $ranks -f $machineFile ./../build/ring_shmem
fi