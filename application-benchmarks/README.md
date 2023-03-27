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
$ cmake -DCMAKE_BUILD_TYPE=Release -DUSE_MPI3 ..
$ make
```
Configuration for Openshmem build
```
$ cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SHMEM ..
$ make
```
Configuration for GASPI build
```
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```
