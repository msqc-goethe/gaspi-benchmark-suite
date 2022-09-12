# gaspi-benchmark

The main objective of this work is to provide a benchmark suite for GASPI to evaluate the performance of implementations such as GPI-2. First, a series of microbenchmarks (Ping Pong, Unidirectional Put, Bidirectional Put, Unidirectional Get, Bidirectional Get, Allreduce Collective, Barrier Collective, Atomics, Notiﬁcation Rate, True One-Sided Exchange, Two-Sided Ping Pong)  are developed that measure the performance of the basic communication routines. 

The microbenchmarks are supplemented by application benchmarks that utilized GASPI in an application context with common access patterns. Both benchmark types are integrated in a uniﬁed suite that provides a simple user interface with ﬁne-grained control over the individual benchmarks.

## Program Usage
To run the GASPI Benchmarking Suite (GBS), a successfully compiled version of the GPI-2 library or another implementation of the GASPI standard is required.
The path to the compiled GPI-2 ﬁles need to be speciﬁed in the ﬁle paths.sh in the scripts directory. After providing the correct path, GBS may be compiled by issuing the make command in the GBS directory. The created binary should be placed in the subdirectory build by default.

The GBS program is intended to be run via the gaspi_run command supplied with GPI-2. However, if only information on the parameters or the benchmarks should be obtained, the GBS program may be launched directly with either the -h parameter to print the program’s usage or with the -list parameter to get a list of all available benchmarks. 

To actually run the benchmark, the program needs to be launched either manually with the help of gaspi_run from the GPI-2 library or with the launch.sh script provided with the GBS ﬁles. Most importantly, a machine ﬁle needs to be created which contains a list of host names with nodes on which the program is spawned. The path of the machine ﬁle is also speciﬁed in paths.sh. With the help of the launch.sh script, all necessary environment variables are set up automatically on the remote nodes, requiring no additional manual conﬁguration.
