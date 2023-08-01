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
## Install GPI-2 through Spack with Slurm Support
First and most important step is to add the Slurm installtion of your system as an external package to spack.
```bash
spack external find slurm
```
This should add the following entry to your ```bash .spack/packages.yaml```file. The entry should look like this:
```bash
slurm:
    externals:
    - spec: slurm@18.08.5
      prefix: /usr
```
If this does not work, try to add it manually.  
Adding external packages should prevent Spack from compiling its own versions of the package. In case of Slurm you would end up with a second Slurm installation which is completely unusable.  
After this you may proceed with the GPI-2 installation. To be on the safe side you may check what will be installed to satisfy the dependencies of the GPI-2 package with ```bash spack spec gpi-2 schedulers=slurm``` and finally ```spack install gpi-2 schedulers=slurm```
