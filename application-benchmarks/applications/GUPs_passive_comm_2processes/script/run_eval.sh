source paths.sh

exePath="$GPI_DIR/bin/gaspi_run"
machineFile="$4"
runfile="wrapper.sh"
gpi_dir=$GPI_DIR
gpi_config=$GPI_CONFIG
buildDir="${HERE}/../build"
exe_name="ra_gaspi"
rfile=$buildDir/$exe_name
chmod +rx $runfile
outfile="$2"
outpath="${HERE}/../results/$outfile"
gaspi_mpi_shmem=$1
ranks="$3"
chmod +rx $runfile
#exe_path is the gaspi_run command
#machine file defines the running nodes
#runfile is the wrapper file, which executes the compiled file
#all the rest: arguments to the wrapper file, pathes to the outputfile,...
if [ $gaspi_mpi_shmem = "gaspi" ] ; then
    echo "RUN GASPI"
    $exePath -m $machineFile  -n 2  $runfile $gpi_dir$gpi_config $rfile  > "$outpath"
fi
if [ $gaspi_mpi_shmem = "mpi" ] ; then
    echo "RUN MPI"
    mpirun -np 2 -f $machineFile ./../build/ra_mpi > "$outpath"
fi
if [ $gaspi_mpi_shmem = "shmem" ] ; then
    echo "RUN SHMEM"
    mpirun -np 2 -f $machineFile ./../build/ra_shmem > "$outpath"
fi