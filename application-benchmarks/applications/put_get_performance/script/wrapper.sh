# As the GPI library is not globlly set in the LD_LIBRARY_PATH Variable
# and GASPI doesnt directly support it, we need to wrap arround
# you can also read the tutorial: https://github.com/cc-hpc-itwm/GPI-2
# -->Environment variables
#LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/diana/gaspi_benchmark/GPI-2/lib/lib64
#$HOME/diana/gaspi_benchmark/gaspi-benchmark/build/ping_pong

#export library path on all nodes (first argument to script)
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$1

#execute the following file on node (second argument to script)
$2 $3

exit $?