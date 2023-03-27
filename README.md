# GASPI BENCHMARK SUITE
In the absence of real GASPI application code, microbench marks and application kernels are the only assessments available to compare and analyze the different features and available implementations. 
The GASPI Benchmark Suite (GBS) is one such collection to evaluate the GASPI standard and the GPI-2 library, the ﬁrst GASPI implementation.

## Build Options

* BUILD_APPS OFF
* BUILD_UBENCH ON

## Build

```
$ mkdir build && cd build
```
Configuraton for micro benchmark
```
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```
Configuration for Application MPI build
```
$ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_APPS=ON -DUSE_MPI3 ..
$ make
```
Configuration for Application Openshmem build
```
$ CC=oshcc; CXX=oshcxx; cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_APPS=ON -DUSE_SHMEM ..
$ make
```
Configuration for Application GPI2 build
```
$ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_APPS=ON ..
$ make
```
