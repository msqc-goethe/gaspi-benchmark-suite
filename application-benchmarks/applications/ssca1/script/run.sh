source paths.sh

exePath="$GPI_DIR/bin/gaspi_run"
machineFile="$MACHINE_FILE"
runfile="wrapper.sh"
gpi_dir=$GPI_DIR
gpi_config=$GPI_CONFIG
buildDir="${HERE}/../build"
exe_name="ssca_gaspi"
rfile=$buildDir/$exe_name
chmod +rx $runfile
gaspi_mpi_shmem=$GASPI_MPI_SHMEM
#exe_path is the gaspi_run command
#machine file defines the running nodes
#runfile is the wrapper file, which executes the compiled file
#all the rest: arguments to the wrapper file, pathes to the outputfile,...
if [ $gaspi_mpi_shmem = "gaspi" ] ; then
    $exePath -m $machineFile  -n 2  $runfile $gpi_dir$gpi_config $rfile # > test.txt
fi
if [ $gaspi_mpi_shmem = "mpi" ] ; then
    mpirun -np 2 -f $machineFile ./../build/ssca_mpi 
fi
if [ $gaspi_mpi_shmem = "shmem" ] ; then
    mpirun -np 2 -f $machineFile ./../build/ssca_shmem
fi