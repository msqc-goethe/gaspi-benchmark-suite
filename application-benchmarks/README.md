# GASPI Application Benchmark

## Applications
* GUPs
* GUPs_large_data
* GUPs_passive_comm_2processes
* GUPs_threaded_different_queues
* put_get_performance
* ring-1d
* ssca1
* ssca1_big_cluster
* ssca1_no_segment_dependancy
* ssca1_threaded_segment

## Build
```
$ mkdir build && cd build
```
Configuration for MPI build
```
$ ../configure CC=$(which mpicc) --enable-mpi --prefix=[INSTALLATIONPATH]
```
Configuration for Openshmem build
```
$ ../configure CC=$(which oshcc) --enable-shmem --prefix=[INSTALLATIONPATH]
```
Configuration for GASPI build
```
$ ../configure --with-gaspi=[PATHTOGASPIBULDDIR] --enable-gaspi --prefix=[INSTALLATIONPATH]
```
or
```
$ ../configure --with-gaspi-include=[PATHTOGASPIINC] --with-gaspi-lib=[PATHTOGASPILIB] --enable-gaspi --prefix=[INSTALLATIONPATH]
```
Explore all configuration options
```
$ ../configure --help
```
After the configuration run:
```
$ make -j
$ make install
```
