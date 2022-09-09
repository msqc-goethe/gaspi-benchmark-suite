/**
* @file gbs_benchmark.c
* @brief Central benchmark sequencing module.
* @detail This file manages the sequential execution
*		  of benchmarks. A list of benchmarks is registered
*		  that can be run. Each benchmark is responsible
*		  for handling the result generation on its own.
* @author Florian Beenen
* @version 2020-11-25
*/

#include "gbs_benchmark.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "success_or_die.h"
#include "gbs_ubench_headers.h"
#include "gbs_abench_grid.h"



#define SEPARATOR_LINE "#------------------------------------------------------------"

/* These are dirty hacks to reuse arraylists
	for function pointers. */
typedef union ptrconv_bench_u {
	void * vptr;
	gbs_benchmark_func fptr;
} ptrconv_bench_t;
static void *f2pConv(gbs_benchmark_func f) {
	ptrconv_bench_t u;
	u.fptr = f;
	return u.vptr;
}
static gbs_benchmark_func p2fConv(void *p) {
	ptrconv_bench_t u;
	u.vptr = p;
	return u.fptr;
}


/**
* This function maps benchmark names to function pointers.
* New benchmarks must be registered here in order to resolve
* them when specified on the command line.
*
* @param str String of the benchmark name.
* @return Function pointer to the resolved benchmark function.
*		  NULL, if the function could not be resolved.
*/
static gbs_benchmark_func mapBenchmarkFunctions(char *str) {
	if (strcmp(str, "gbs_ubench_ping_pong") == 0) {
		return gbs_ubench_ping_pong;
	} else if (strcmp(str, "gbs_ubench_put_single_udir") == 0) {
		return gbs_ubench_put_single_udir;
	} else if (strcmp(str, "gbs_ubench_put_single_bdir") == 0) {
		return gbs_ubench_put_single_bdir;
	} else if (strcmp(str, "gbs_ubench_put_all_udir") == 0) {
		return gbs_ubench_put_all_udir;
	} else if (strcmp(str, "gbs_ubench_put_all_bdir") == 0) {
		return gbs_ubench_put_all_bdir;
	} else if (strcmp(str, "gbs_ubench_put_solo_single_udir") == 0) {
		return gbs_ubench_put_solo_single_udir;
	} else if (strcmp(str, "gbs_ubench_put_aggregate_single_udir") == 0) {
		return gbs_ubench_put_aggregate_single_udir;
	} else if (strcmp(str, "gbs_ubench_get_single_udir") == 0) {
		return gbs_ubench_get_single_udir;
	} else if (strcmp(str, "gbs_ubench_get_single_bdir") == 0) {
		return gbs_ubench_get_single_bdir;
	} else if (strcmp(str, "gbs_ubench_get_all_udir") == 0) {
		return gbs_ubench_get_all_udir;
	} else if (strcmp(str, "gbs_ubench_get_all_bdir") == 0) {
		return gbs_ubench_get_all_bdir;
	} else if (strcmp(str, "gbs_ubench_barrier") == 0) {
		return gbs_ubench_barrier;
	} else if (strcmp(str, "gbs_ubench_twosided_ping_pong") == 0) {
		return gbs_ubench_twosided_ping_pong;
	} else if (strcmp(str, "gbs_ubench_allreduce_min") == 0) {
		return gbs_ubench_allreduce_min;
	} else if (strcmp(str, "gbs_ubench_allreduce_max") == 0) {
		return gbs_ubench_allreduce_max;
	} else if (strcmp(str, "gbs_ubench_allreduce_sum") == 0) {
		return gbs_ubench_allreduce_sum;
	} else if (strcmp(str, "gbs_ubench_atomic_fetch_add_single") == 0) {
		return gbs_ubench_atomic_fetch_add_single;
	} else if (strcmp(str, "gbs_ubench_atomic_fetch_add_all") == 0) {
		return gbs_ubench_atomic_fetch_add_all;
	} else if (strcmp(str, "gbs_ubench_atomic_compare_swap_single") == 0) {
		return gbs_ubench_atomic_compare_swap_single;
	} else if (strcmp(str, "gbs_ubench_atomic_compare_swap_all") == 0) {
		return gbs_ubench_atomic_compare_swap_all;
	} else if (strcmp(str, "gbs_ubench_noti_single_udir") == 0) {
		return gbs_ubench_noti_single_udir;
	} else if (strcmp(str, "gbs_ubench_noti_single_bdir") == 0) {
		return gbs_ubench_noti_single_bdir;
	} else if (strcmp(str, "gbs_ubench_noti_all_udir") == 0) {
		return gbs_ubench_noti_all_udir;
	} else if (strcmp(str, "gbs_ubench_noti_all_bdir") == 0) {
		return gbs_ubench_noti_all_bdir;
	} else if (strcmp(str, "gbs_ubench_put_true_exchange") == 0) {
		return gbs_ubench_put_true_exchange;
	} else if (strcmp(str, "gbs_ubench_get_true_exchange") == 0) {
		return gbs_ubench_get_true_exchange;
	} else if (strcmp(str, "gbs_abench_grid_stencil") == 0) {
		return gbs_abench_grid_stencil;
	}
	return NULL;
}

/**
* Creates a list with the names of all known benchmarks.
* Add new benchmarks here in order to automatically execute
* them in the default benchmark run or show them with the
* '-list' command line argument.
*
* @return List with the strings of all known benchmarks.
*/
static arraylist_t *getBenchmarkStringList(void) {
	arraylist_t *r = arraylist_create();
	arraylist_add(r, "gbs_ubench_ping_pong");
	arraylist_add(r, "gbs_ubench_put_single_udir");
	arraylist_add(r, "gbs_ubench_put_single_bdir");
	arraylist_add(r, "gbs_ubench_put_all_udir");
	arraylist_add(r, "gbs_ubench_put_all_bdir");
	arraylist_add(r, "gbs_ubench_put_solo_single_udir");
	arraylist_add(r, "gbs_ubench_put_aggregate_single_udir");
	arraylist_add(r, "gbs_ubench_get_single_udir");
	arraylist_add(r, "gbs_ubench_get_single_bdir");
	arraylist_add(r, "gbs_ubench_get_all_udir");
	arraylist_add(r, "gbs_ubench_get_all_bdir");
	arraylist_add(r, "gbs_ubench_barrier");
	arraylist_add(r, "gbs_ubench_twosided_ping_pong");
	arraylist_add(r, "gbs_ubench_allreduce_min");
	arraylist_add(r, "gbs_ubench_allreduce_max");
	arraylist_add(r, "gbs_ubench_allreduce_sum");
	arraylist_add(r, "gbs_ubench_atomic_fetch_add_single");
	arraylist_add(r, "gbs_ubench_atomic_fetch_add_all");
	arraylist_add(r, "gbs_ubench_atomic_compare_swap_single");
	arraylist_add(r, "gbs_ubench_atomic_compare_swap_all");
	arraylist_add(r, "gbs_ubench_noti_single_udir");
	arraylist_add(r, "gbs_ubench_noti_single_bdir");
	arraylist_add(r, "gbs_ubench_noti_all_udir");
	arraylist_add(r, "gbs_ubench_noti_all_bdir");
	arraylist_add(r, "gbs_ubench_put_true_exchange");
	arraylist_add(r, "gbs_ubench_get_true_exchange");
	arraylist_add(r, "gbs_abench_grid_stencil");
	return r;
}

static void fillBenchmarkFunctionList(arraylist_t *targetList) {
	arraylist_t *blist = getBenchmarkStringList();
	boolean ok = gbs_parseBenchmarkFunctions(blist, targetList);
	assert (ok);
	blist = arraylist_delete(blist);
}

boolean gbs_parseBenchmarkFunctions(arraylist_t *stringList, arraylist_t *functionList) {
	unsigned int i;
	boolean parseOk = true;
	assert(functionList != NULL);
	if (stringList != NULL) {
		for (i = 0; i < arraylist_getLength(stringList); ++i) {
			char *fname = arraylist_get(stringList, i);
			gbs_benchmark_func func = mapBenchmarkFunctions(fname);
			parseOk &= (func != NULL);
			if (func != NULL) {
				arraylist_add(functionList, f2pConv(func));
			}
		}
	}
	return parseOk;
}

static void printOutputPreamble(FILE *f, cmdArgs_t *cmdArgs, unsigned int nproc ) {
	time_t t1 = time(NULL);
	struct tm t = *localtime(&t1);
	fprintf(f, "%s\n", SEPARATOR_LINE);
	fprintf(f, "# GASPI Benchmark Suite by Florian Beenen\n");
	fprintf(f, "# Date:  %d-%02d-%02d %02d:%02d:%02d\n", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	fprintf(f, "# Maximum repetitions: %d\n", cmdArgs->maxRuns);
	fprintf(f, "# Minimum repetitions: %d\n", cmdArgs->minRuns);
	fprintf(f, "# Block Aggregation: %d\n", cmdArgs->aggregateCount);
	fprintf(f, "# Warmup runs: %d%c\n", cmdArgs->warmupPercent, '%');
	fprintf(f, "# Repetition reduction threshold: %d bytes\n", cmdArgs->runReduceThres);
	fprintf(f, "# Start transfer size: %d bytes\n", cmdArgs->sizeStart);
	fprintf(f, "# Last transfer size: %d bytes\n", cmdArgs->sizeEnd);
	fprintf(f, "# Transfer size increase factor: %d\n", cmdArgs->sizeFactor);
	fprintf(f, "# Average results over all nodes: %s\n", cmdArgs->timeCombined ? "true" : "false");
	fprintf(f, "# Grid Benchmark - Grid start size: %d\n", cmdArgs->gridSizeStart);
	fprintf(f, "# Grid Benchmark - Grid last size: %d\n", cmdArgs->gridSizeEnd);
	fprintf(f, "# Grid Benchmark - Grid size increase factor: %d\n", cmdArgs->gridSizeFactor);
	fprintf(f, "# Grid Benchmark - Iterations on one grid: %d\n", cmdArgs->gridIterations);
	fprintf(f, "# Grid Benchmark - Repetition reduce threshold after grid size: %d\n", cmdArgs->gridReduceThres);
	fprintf(f, "# Grid Benchmark - Minimum repetitions: %d\n", cmdArgs->gridMinRuns);
	fprintf(f, "# Grid Benchmark - Maximum repetitions: %d\n", cmdArgs->gridMaxRuns);
	fprintf(f, "# Grid Benchmark - Processing Threads: %d\n", cmdArgs->gridThreads);
	fprintf(f, "# Number of processes: %d\n", nproc);
	fprintf(f, "%s\n", SEPARATOR_LINE);
	fflush(f);
}


static void printBenchmarkList(FILE *f, arraylist_t *benchmarkStringList) {
	unsigned int i;
	fprintf(f, "# List of Benchmarks to run:\n");
	for (i = 0; i < arraylist_getLength(benchmarkStringList); ++i) {
		fprintf(f, "# %s\n", (char *) arraylist_get(benchmarkStringList, i));
	}
}


static void printBenchmarkPreamble(FILE *f, char *benchName) {
	fprintf(f, "%s\n", SEPARATOR_LINE);
	fprintf(f, "# Benchmarking %s\n", benchName);
	fprintf(f, "%s\n", SEPARATOR_LINE);
}


/**
* Get a list of the string representations of the given list with
* benchmark functions. This function filters the list of all benchmarks
* for the names that correspond to the given function pointers.
* The ordering of the strings is equal to the ordering in the function
* pointer list.
*
* @param benchmarkFunctions List with function pointers of type 'gbs_benchmark_func'.
* @return List with static strings with the names of the benchmarks.
*/
static arraylist_t *getBenchmarkStringNames(arraylist_t *benchmarkFunctions) {
	unsigned int i;
	arraylist_t *result = arraylist_create();
	arraylist_t *blist = getBenchmarkStringList();
	arraylist_t *allFunctionMappings = arraylist_create();
	boolean ok = gbs_parseBenchmarkFunctions(blist, allFunctionMappings);
	assert(ok);
	for (i = 0; i < arraylist_getLength(benchmarkFunctions); ++i) {
		void *currentFunc = arraylist_get(benchmarkFunctions, i);
		unsigned int j = 0;
		boolean found = false;
		for (j = 0; j < arraylist_getLength(allFunctionMappings) && !found; ++j) {
			void *otherFunc = arraylist_get(allFunctionMappings, j);
			found = (currentFunc == otherFunc);
			if (found) {
				arraylist_add(result, arraylist_get(blist, j)); /* Add String representation */
			}
		}
		assert(found);
	}

	blist = arraylist_delete(blist);
	allFunctionMappings = arraylist_delete(allFunctionMappings);
	return result;
}

boolean gbs_executeBenchmarks(FILE *outputStream, cmdArgs_t *args, arraylist_t *benchListFunc) {
	unsigned int benchmarkIdx;
	gaspi_rank_t iproc;
	gaspi_rank_t nproc;
	arraylist_t *benchmarkStringList = NULL;

	if (arraylist_isEmpty(benchListFunc)) {
		fillBenchmarkFunctionList(benchListFunc);
	}
	benchmarkStringList = getBenchmarkStringNames(benchListFunc);

	SUCCESS_OR_DIE(gaspi_proc_rank(&iproc));
	SUCCESS_OR_DIE(gaspi_proc_num(&nproc));

	assert(nproc >= 2 && "Benchmarks need to be run with at least 2 nodes");

	if (iproc == 0) {
		printOutputPreamble(outputStream, args, nproc);
		fprintf(outputStream, "\n");
		printBenchmarkList(outputStream, benchmarkStringList);
	}

	/* Run the Benchmarks */
	for (benchmarkIdx = 0; benchmarkIdx < arraylist_getLength(benchListFunc); ++benchmarkIdx) {
		gbs_benchmark_func benchFunc = p2fConv(arraylist_get(benchListFunc, benchmarkIdx));
		char *benchmarkName = ((char *) arraylist_get(benchmarkStringList, benchmarkIdx));
		gbs_bench_config_t conf;
		conf.outputStream = outputStream;
		conf.cmdArgs = args;
		conf.iproc = iproc;
		conf.nproc = nproc;
		if (iproc == 0) {
			fprintf(outputStream, "\n");
			printBenchmarkPreamble(outputStream, benchmarkName);
		}
		benchFunc(&conf); /* Call the benchmark function. It should also handle output-printing. */
	}
	benchmarkStringList = arraylist_delete(benchmarkStringList);
	return true;
}

void gbs_printBenchmarkList(void) {
	unsigned int i;
	arraylist_t *blist = getBenchmarkStringList();
	printf("# Benchmark List\n#\n");
	for (i = 0; i < arraylist_getLength(blist); ++i) {
		printf("# %s\n", (char *)arraylist_get(blist, i));
	}
	blist = arraylist_delete(blist);
}

