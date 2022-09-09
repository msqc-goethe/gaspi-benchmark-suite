source paths.sh

exePath="$GPI_DIR/bin/gaspi_run"
machineFile="$4"
runfile="wrapper.sh"
gpi_dir=$GPI_DIR
gpi_config=$GPI_CONFIG
buildDir="${HERE}/../build"
exe_name="ssca_gaspi"
rfile=$buildDir/$exe_name
thread_num="$5"
rank="$3"
chmod +rx $runfile
outfile="$2"
outpath="${HERE}/../results/$outfile"
gaspi_mpi_shmem=$1
#exe_path is the gaspi_run command
#machine file defines the running nodes
#runfile is the wrapper file, which executes the compiled file
#all the rest: arguments to the wrapper file, pathes to the outputfile,...

if [ $gaspi_mpi_shmem = "gaspi" ] ; then
    $exePath -m $machineFile  -n $rank  $runfile $gpi_dir$gpi_config $rfile $thread_num  > "$outpath"
fi
if [ $gaspi_mpi_shmem = "mpi" ] ; then
    mpirun -np $rank  -f $machineFile ./../build/ssca_mpi $thread_num $scale > "$outpath"
fi
if [ $gaspi_mpi_shmem = "shmem" ] ; then
    mpirun -np $rank  -f $machineFile ./../build/ssca_shmem $thread_num $scale > "$outpath"
fi