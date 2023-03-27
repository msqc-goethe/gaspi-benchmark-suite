# Random Access Benchmark
This is a modified version of the Random Access Benchmark initially developed by David Koester and Bob Lucas.
Basic requirements of the RandomAccess benchmark are:
 (1) size of the table T[ ] being a power of two and approximately half the global memory and with 
 (2) look-ahead and storage constraints being followed. Specifically, attempts to improve memory access characteristics by reordering the address stream -- so that all updates are "local" -- violates the rules of the RandomAccess benchmark.

In the OpenSHMEM version the entire HPCC_Table is allocated via shmem_malloc and all update locations are ensured to be both random and remote.
__________________________________________________________________________________________________
This is a modified version of the Oak Ridge OpenSHMEM Benchmark Suite/ Random Access developed by Thomas Naughton, Ferrol Aderholdt, Matt Baker, Swaroop Pophale, Manjunath Gorentla Venkata and Neena Imam
It contains a mpi, gaspi and (** IT DOES NOT CONTAIN AN OpenSHMEM version**).
