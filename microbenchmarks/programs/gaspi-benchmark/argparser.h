/**
* @file argparser.h
* @brief Functions to process the command line arguments.
* @detail This file contains functions that parse the command
*	      line aguments and return the parsed information in a
*		  custom struct.
* @author Florian Beenen
* @version 2020-11-24
*/

#ifndef __ARGPARSER_H__
#define __ARGPARSER_H__

#include <stdio.h>
#include <utility.h>
#include <arraylist.h>

#define ARG_IDEN_TEST					"-test"
#define ARG_IDEN_TIME_COMBINED			"-time-combined"
#define ARG_IDEN_BENCH_LIST				"-list"
#define ARG_IDEN_BENCH					"-bench"
#define ARG_IDEN_MAXRUNS				"-maxruns"
#define ARG_IDEN_MINRUNS				"-minruns"
#define ARG_IDEN_RUN_REDUCE_THRES		"-reducethres"
#define ARG_IDEN_AGGREGATE				"-aggregate"
#define ARG_IDEN_WARMUP					"-warmup"
#define ARG_IDEN_SIZE_START				"-size-start"
#define ARG_IDEN_SIZE_END 				"-size-stop"
#define ARG_IDEN_SIZE_FACTOR 			"-size-factor"
#define ARG_IDEN_GRID_SIZE_START		"-grid-size-start"
#define ARG_IDEN_GRID_SIZE_END			"-grid-size-stop"
#define ARG_IDEN_GRID_SIZE_FACTOR		"-grid-size-factor"
#define ARG_IDEN_GRID_ITERATIONS		"-grid-iterations"
#define ARG_IDEN_GRID_REDUCE_THRES		"-grid-reducethres"
#define ARG_IDEN_GRID_MINRUNS			"-grid-minruns"
#define ARG_IDEN_GRID_MAXRUNS			"-grid-maxruns"
#define ARG_IDEN_GRID_THREADS			"-grid-threads"
#define ARG_IDEN_HELP					"-h"

#define ARG_DEFAULT_TIME_COMBINED			false
#define ARG_DEFAULT_MAXRUNS					1024
#define ARG_DEFAULT_MINRUNS					10
#define ARG_DEFAULT_RUN_REDUCE_THRES		65536
#define ARG_DEFAULT_AGGREGATE				1
#define ARG_DEFAULT_WARUMUP					10
#define ARG_DEFAULT_SIZE_START				1
#define ARG_DEFAULT_SIZE_END				4194304
#define ARG_DEFAULT_SIZE_FACTOR				2
#define ARG_DEFAULT_GRID_SIZE_START			16
#define ARG_DEFAULT_GRID_SIZE_END			2048
#define ARG_DEFAULT_GRID_SIZE_FACTOR		2
#define ARG_DEFAULT_GRID_ITERATIONS			128
#define ARG_DEFAULT_GRID_REDUCE_THRES		512
#define ARG_DEFAULT_GRID_MINRUNS			10
#define ARG_DEFAULT_GRID_MAXRUNS			128
#define ARG_DEFAULT_GRID_THREADS			1


typedef struct argParseStatus_s {
	boolean parseOk;
	char *invalidParameterName;
} argParseStatus_t;


/**
* Struct that contains the parsed arguments.
*/
typedef struct cmdArgs_s {
	arraylist_t *benchlistStrings;
	unsigned int maxRuns;
	unsigned int minRuns;
	unsigned int runReduceThres;
	unsigned int aggregateCount;
	unsigned int warmupPercent;
	unsigned int sizeStart;
	unsigned int sizeEnd;
	unsigned int sizeFactor;
	unsigned int gridSizeStart;
	unsigned int gridSizeEnd;
	unsigned int gridSizeFactor;
	unsigned int gridIterations;
	unsigned int gridReduceThres;
	unsigned int gridMinRuns;
	unsigned int gridMaxRuns;
	unsigned int gridThreads;
	boolean timeCombined;
	boolean listBenchmarks;
	boolean showHelp;
} cmdArgs_t;

/**
* Function that parses the command line arguments. The parameters
* found are returned in the given struct.
*
* @param argc Number of command line arguments.
* @param argv The command line arguments
* @param args Output parameter for returning the parsed arguments. Memory
*		 does not need to be initialized.
* @return PARSE_OK, if everything is OK. Otherwise, a error-specific code
*		  is returned.
*/
extern void argparser_parse(int argc, char ** argv, cmdArgs_t *args, argParseStatus_t *status);

/**
* Function to print a human-readable error message for the error
* code obtained when parsing command line arguments. The message
* is printed to stderr.
*
* @param status Status code of the parsing function.
*/
extern void argparser_printErr(argParseStatus_t *status);

/**
* Display the program's help information on the command line.
*
* @param progName The name of the progam (i.e. argv[0]).
*/
extern void argparser_showHelp(char *progName);

/**
* Frees potential dynamic datastructures in the 'cmdArgs_t' 
* struct.
* @param args The parsed struct. Should not be touched after 
*		 callig this function. 
*/
extern void argparser_free(cmdArgs_t *args);

#endif
