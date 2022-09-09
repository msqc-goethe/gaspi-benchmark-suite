source paths.sh

exePath="$GPI_DIR/bin/gaspi_run"
machineFile=$MACHINE_FILE
runfile=$GASPI_RUN_FILE
gpi_dir=$GPI_DIR
gpi_config=$GPI_CONFIG
build_dir="$HERE/$BUILD_DIR"
exe_name=$EXE_NAME


chmod +rx $runfile
#exe_path is the gaspi_run command
#machine file defines the running nodes
#runfile is the wrapper file, which executes the compiled file
#all the rest: arguments to the wrapper file, pathes to the outputfile,...
$exePath -m $machineFile -n 2 $runfile $gpi_dir$gpi_config $build_dir/$exe_name > test.txt