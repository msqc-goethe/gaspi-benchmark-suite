/**
* @file gbs_benchmark.h
* @brief Central benchmark sequencing module.
* @detail This file manages the sequential execution
*		  of benchmarks. A list of benchmarks is registered
*		  that can be run. Each benchmark is responsible
*		  for handling the result generation on its own.
* @author MSQC
* @version 2020-11-25
*/

#ifndef __GBS_BENCHMARK_H__
#define __GBS_BENCHMARK_H__

#include <GASPI.h>
#include <utility.h>
#include <arraylist.h>
#include "argparser.h"

/**
* Configuration data for each benchmark. Data
* is constant over all benchmarks.
*/
typedef struct gbs_bench_config_s {
	cmdArgs_t *cmdArgs;
	FILE *outputStream;
	gaspi_rank_t iproc;
	gaspi_rank_t nproc;
} gbs_bench_config_t;



/**
* Function Interface for all GASPI Functions.
*/
typedef void (*gbs_benchmark_func)(gbs_bench_config_t *);

/**
* Parses a list of strings and tries to map the given string contents
* to benchmark names. All correctly parsed names are converted into
* function pointers.
*
* @param stringList List with strings that contain possible benchmark names.
* @param functionList Allocated empty list in which the function pointers of
* type 'gbs_benchmark_func' are stored that are successfully parsed.
* @return true, if all given names were parsed correctly. False, otherwise.
*/
boolean gbs_parseBenchmarkFunctions(arraylist_t *stringList, arraylist_t *functionList);

/**
* Executes the given list of benchmarks in a sequential fashion.
*
* @param outputStream The stream for printing the results.
* @param args The parsed command-line arguments.
* @param benchListFunc List with 'gbs_benchmark_func' of benchmark functions
*		that are executed.
* @return True, if the execution was successful. False, otherwise.
*/
boolean gbs_executeBenchmarks(FILE *outputStream, cmdArgs_t *args, arraylist_t *benchListFunc);

/**
* Gather a list of all known benchmarks and print them to stdout.
* Can be executed without active GASPI environment.
*/
void gbs_printBenchmarkList(void);


#endif
