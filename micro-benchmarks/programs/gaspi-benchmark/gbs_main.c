/**
* @file gbs_main.c
* @brief Main program for the GASPI Benchmarking Suite.
* @detail The GASPI Benchmarking Suite runs a series of
*		  microbenchmarks and application benchmarks to
*		  evaluate the performance of GASPI for a specific
*		  implementation.
*		  The results are provided in the same way as in
*		  Intel MPI Benchmark (IMB).
* @author MSQC
* @version 2020-11-24
*/

#include <stdio.h>
#include <stdlib.h>
#include <arraylist.h>
#include <GASPI.h>
#include "argparser.h"
#include "success_or_die.h"
#include "gbs_benchmark.h"


/**
* Checks, if multiple processes are launched. If the
* program is launched without GASPI Runtime, initOK will
* have failed and therefore only one process is active.
* If GASPI Runtime is running, only Rank 0 will return
* true
*
* @param initOk The return code of the gaspi_proc_init
*		 function.
* @return true for the first process in the GASPI environment,
*		  false otherwise. 
*/
static boolean isRankZero(gaspi_return_t initOk) {
	if (initOk == GASPI_SUCCESS) {
		gaspi_rank_t iproc;
		SUCCESS_OR_DIE(gaspi_proc_rank(&iproc));
		return (iproc == 0);
	}
	return true;
}

int main(int argc, char *argv[]) {
	cmdArgs_t args;
	argParseStatus_t argStatus;
	gaspi_return_t initOk;
	boolean ok = true;
	boolean isRank0;
	int returnCode = 0;
	FILE *outputStream = stdout;
	arraylist_t *benchmarkFunctions = arraylist_create();
	
	initOk = gaspi_proc_init(GASPI_BLOCK);
	isRank0 = isRankZero(initOk);
	argparser_parse(argc, argv, &args, &argStatus);
	if (!argStatus.parseOk)  {
		if (isRank0) {
			argparser_printErr(&argStatus);
		}
		returnCode = 1;
	} else if (args.showHelp) {
		if (isRank0) {
			argparser_showHelp(argv[0]);
		}
	} else if (args.listBenchmarks) {
		if (isRank0) {
			gbs_printBenchmarkList();
		}
	} else {
		SUCCESS_OR_DIE(initOk);
	
		ok &= gbs_parseBenchmarkFunctions(args.benchlistStrings, benchmarkFunctions);
		if (ok)  {
			ok = gbs_executeBenchmarks(outputStream, &args, benchmarkFunctions); 
			if (!ok) {
				fprintf(stderr, "Benchmark failed\n");
				returnCode = 3;
			}
		} else {
			fprintf(stderr, "Not all given benchmarks represent valid benchmark names.\n");
			returnCode = 2;
		}

		SUCCESS_OR_DIE(gaspi_proc_term(GASPI_BLOCK));
	}
	benchmarkFunctions = arraylist_delete(benchmarkFunctions);
	argparser_free(&args);
	return returnCode;
}
