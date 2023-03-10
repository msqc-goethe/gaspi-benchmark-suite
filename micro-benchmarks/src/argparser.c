/**
* @file argparser.c
* @brief Functions to process the command line arguments.
* @detail This file contains functions that parse the command
*	      line aguments and return the parsed information in a
*		  custom struct.
* @author Florian Beenen
* @version 2020-11-24
*/

#include "argparser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef enum argparse_value_comparator_e {
	VALCOMP_EVERYTHING,
	VALCOMP_GREATER_ZERO,
	VALCOMP_GREATER_EQUAL_ZERO,
	VALCOMP_GREATER_ONE
} argparse_value_comparator_t;

/*
* Find the string after the identifier.
*/
static char *parseString(char *argvi, char *identifier) {
	char *string = strstr(argvi, identifier);
	if (string != NULL) {
		int argLen = strlen(identifier);
		string += argLen; /* Must succeed. */
		if (*string == '=') { /* We found the right position */
			return string + 1;
		}
	}
	return NULL;
}

static arraylist_t *splitCommaString(char *str) {
	arraylist_t *result = arraylist_create();
	unsigned int start = 0;
	unsigned int i;
	unsigned int len;
	len = strlen(str);
	for (i = 0; i < len; ++i) {
		char c = str[i];
		if (c == ',') {
			char *substr = stringSubstring(str, start, i);
			arraylist_add(result, substr);
			start = i + 1;
		}
	}
	if (i > start) { /* There was an element at the end */
		char *substr = stringSubstring(str, start, len);
		arraylist_add(result, substr);
	}
	return result;
}

static void parseUInt(argParseStatus_t *status, char *parameterName, char *str, argparse_value_comparator_t comparator, unsigned int *out) {
	assert(status != NULL);
	if (status->parseOk) {
		char *found;
		found = strstr(str, parameterName);
		if (found == str) { /* if string starts with the parameter name */
			long value;
			int scanres;
			scanres = sscanf(str + strlen(parameterName), "=%ld", &value);
			if (scanres == 1) {
				boolean ok = (
						( (comparator == VALCOMP_GREATER_EQUAL_ZERO) && (value >= 0) ) ||
						( (comparator == VALCOMP_GREATER_ZERO) && (value > 0) ) ||
						( (comparator == VALCOMP_GREATER_ONE) && (value > 1) ) ||
						(comparator == VALCOMP_EVERYTHING));
				if (!ok) {
					status->parseOk = false;
					status->invalidParameterName = parameterName;
				}
				*out = (unsigned int) value;
			}
		}
	}
}

extern void argparser_parse(int argc, char ** argv, cmdArgs_t *args, argParseStatus_t *status) {
    char *string = NULL;
	int i;
	boolean testMode = false;
	args->benchlistStrings = NULL;
    args->maxRuns = ARG_DEFAULT_MAXRUNS;
    args->minRuns = ARG_DEFAULT_MINRUNS;
    args->runReduceThres = ARG_DEFAULT_RUN_REDUCE_THRES;
    args->aggregateCount = ARG_DEFAULT_AGGREGATE;
    args->warmupPercent = ARG_DEFAULT_WARUMUP;
    args->sizeStart = ARG_DEFAULT_SIZE_START;
    args->sizeEnd = ARG_DEFAULT_SIZE_END;
    args->sizeFactor = ARG_DEFAULT_SIZE_FACTOR;
    args->gridSizeStart = ARG_DEFAULT_GRID_SIZE_START;
    args->gridSizeEnd = ARG_DEFAULT_GRID_SIZE_END;
    args->gridSizeFactor = ARG_DEFAULT_GRID_SIZE_FACTOR;
    args->gridIterations = ARG_DEFAULT_GRID_ITERATIONS;
    args->gridReduceThres = ARG_DEFAULT_GRID_REDUCE_THRES;
    args->gridMinRuns = ARG_DEFAULT_GRID_MINRUNS;
    args->gridMaxRuns = ARG_DEFAULT_GRID_MAXRUNS;
    args->gridThreads = ARG_DEFAULT_GRID_THREADS;
    args->timeCombined = ARG_DEFAULT_TIME_COMBINED;
	args->listBenchmarks = false;
    args->showHelp = false;
	
	status->parseOk = true;
	status->invalidParameterName = NULL;
	for (i = 1; i < argc; ++i) {
		parseUInt(status, ARG_IDEN_MAXRUNS, argv[i], VALCOMP_GREATER_ZERO, &(args->maxRuns));
		parseUInt(status, ARG_IDEN_MINRUNS, argv[i], VALCOMP_GREATER_ZERO, &(args->minRuns));
		parseUInt(status, ARG_IDEN_AGGREGATE, argv[i], VALCOMP_GREATER_ZERO, &(args->aggregateCount));
		parseUInt(status, ARG_IDEN_RUN_REDUCE_THRES, argv[i], VALCOMP_GREATER_EQUAL_ZERO, &(args->runReduceThres));
		parseUInt(status, ARG_IDEN_WARMUP, argv[i], VALCOMP_GREATER_EQUAL_ZERO, &(args->warmupPercent));
		if (args->warmupPercent > 50) {
			status->parseOk = false;
			status->invalidParameterName = ARG_IDEN_WARMUP;
		}
		parseUInt(status, ARG_IDEN_SIZE_START, argv[i], VALCOMP_GREATER_ZERO, &(args->sizeStart));
		parseUInt(status, ARG_IDEN_SIZE_END, argv[i], VALCOMP_GREATER_ZERO, &(args->sizeEnd));
		parseUInt(status, ARG_IDEN_SIZE_FACTOR, argv[i], VALCOMP_GREATER_ONE, &(args->sizeFactor));
		parseUInt(status, ARG_IDEN_GRID_SIZE_START, argv[i], VALCOMP_GREATER_ONE, &(args->gridSizeStart));
		parseUInt(status, ARG_IDEN_GRID_SIZE_END, argv[i], VALCOMP_GREATER_ONE, &(args->gridSizeEnd));
		parseUInt(status, ARG_IDEN_GRID_SIZE_FACTOR, argv[i], VALCOMP_GREATER_ONE, &(args->gridSizeFactor));
		parseUInt(status, ARG_IDEN_GRID_ITERATIONS, argv[i], VALCOMP_GREATER_ZERO, &(args->gridIterations));
		parseUInt(status, ARG_IDEN_GRID_REDUCE_THRES, argv[i], VALCOMP_GREATER_EQUAL_ZERO, &(args->gridReduceThres));
		parseUInt(status, ARG_IDEN_GRID_MINRUNS, argv[i], VALCOMP_GREATER_ZERO, &(args->gridMinRuns));
		parseUInt(status, ARG_IDEN_GRID_MAXRUNS, argv[i], VALCOMP_GREATER_ZERO, &(args->gridMaxRuns));
		parseUInt(status, ARG_IDEN_GRID_THREADS, argv[i], VALCOMP_GREATER_ZERO, &(args->gridThreads));

		string = parseString(argv[i], ARG_IDEN_BENCH);
		if (string != NULL) {
			args->benchlistStrings = splitCommaString(string);
		}
	
		if (strcmp(ARG_IDEN_TEST, argv[i]) == 0) {
			testMode = true;
		}
		if (strcmp(ARG_IDEN_TIME_COMBINED, argv[i]) == 0) {
			args->timeCombined = true;
		}
		if (strcmp(ARG_IDEN_BENCH_LIST, argv[i]) == 0) {
			args->listBenchmarks = true;
		}
		if (strcmp(ARG_IDEN_HELP, argv[i]) == 0) {
			args->showHelp = true;
		}
	}
	if (testMode) { /* Overwrite previous data */
		args->gridMaxRuns = 1;
		args->gridMinRuns = 1;
		args->gridIterations = 1;
		args->warmupPercent = 0;
		args->maxRuns = 1;
		args->minRuns = 1;
		args->aggregateCount = 1;
	}
}

extern void argparser_printErr(argParseStatus_t *status) {
	if (status->parseOk) {
		fprintf(stderr, "Argument parsing successful\n");
	} else {
		 fprintf(stderr, "Error parsing arguments. Invalid value given for argument %s.\n", status->invalidParameterName);
	}
}

/*
* Displays usage information.
*/
extern void argparser_showHelp(char *progName) {
    printf("USAGE for \"%s\"\n", progName);
    printf("###########################\n");
    printf(ARG_IDEN_BENCH"=VALUE[,VALUE,...,VALUE] : specify a list of benchmarks to run (optional).\n");
    printf(ARG_IDEN_MAXRUNS"=VALUE: specify the maximal number of repetitions in one single test (Default: %d).\n", ARG_DEFAULT_MAXRUNS);
    printf(ARG_IDEN_MINRUNS"=VALUE: specify the minimal number of repetitions in one single test (Default: %d).\n", ARG_DEFAULT_MINRUNS);
    printf(ARG_IDEN_AGGREGATE"=VALUE: specify the number of transfers to aggregate before flush (Default: %d).\n", ARG_DEFAULT_AGGREGATE);
    printf(ARG_IDEN_RUN_REDUCE_THRES"=VALUE: specify the transfer size in bytes from which to reduce the number of repetitions. Zero disables this (Default: %d).\n", ARG_DEFAULT_RUN_REDUCE_THRES);
    printf(ARG_IDEN_WARMUP"=VALUE: specify the percentage of warm-up repetitions before the actual measurement (Default: %d%c).\n", ARG_DEFAULT_WARUMUP, '%');
    printf(ARG_IDEN_SIZE_START"=VALUE: specify the number of bytes to transfer as the first data point (Default: %d).\n", ARG_DEFAULT_SIZE_START);
    printf(ARG_IDEN_SIZE_END"=VALUE: specify the number of bytes to transfer as the last data point (Default: %d).\n", ARG_DEFAULT_SIZE_END);
    printf(ARG_IDEN_SIZE_FACTOR"=VALUE: specify the factor to increase the transfer size with (Default: %d).\n", ARG_DEFAULT_SIZE_FACTOR);
    printf(ARG_IDEN_GRID_SIZE_START"=VALUE: Application benchmark Grid: Specify the starting size of the grid (Default: %d).\n", ARG_DEFAULT_GRID_SIZE_START);
    printf(ARG_IDEN_GRID_SIZE_END"=VALUE: Application benchmark Grid: Specify the final size of the grid (Default: %d).\n", ARG_DEFAULT_GRID_SIZE_END);
    printf(ARG_IDEN_GRID_SIZE_FACTOR"=VALUE: Application benchmark Grid: Specify the factor to increase the grid size with (Default: %d).\n", ARG_DEFAULT_GRID_SIZE_FACTOR);
    printf(ARG_IDEN_GRID_ITERATIONS"=VALUE: Application benchmark Grid: Specify the number of time steps to perform for one single measurement (Default: %d).\n", ARG_DEFAULT_GRID_ITERATIONS);
    printf(ARG_IDEN_GRID_REDUCE_THRES"=VALUE: Application benchmark Grid: Specify the threshold for the grid size from which the number of measurement repetitions is reduced. Zero disables this (Default: %d).\n", ARG_DEFAULT_GRID_REDUCE_THRES);
    printf(ARG_IDEN_GRID_MINRUNS"=VALUE: Application benchmark Grid: Specify the minimal number of repetitions for one grid size (Default: %d).\n", ARG_DEFAULT_GRID_MINRUNS);
    printf(ARG_IDEN_GRID_MAXRUNS"=VALUE: Application benchmark Grid: Specify the maximal number of repetitions for one grid size (Default: %d).\n", ARG_DEFAULT_GRID_MAXRUNS);
    printf(ARG_IDEN_GRID_THREADS"=VALUE: Application benchmark Grid: Specify the number of processing threads (Default: %d).\n", ARG_DEFAULT_GRID_THREADS);
    printf(ARG_IDEN_TIME_COMBINED": Fetch time measurements from all nodes and report the average instead of only the result from rank 0 (Default: %s).\n", (ARG_DEFAULT_TIME_COMBINED ? "true" : "false"));
    printf(ARG_IDEN_TEST": Run benchmarks with the minimum number of repetitions or iterations.\n");
    printf(ARG_IDEN_BENCH_LIST": List the number of available benchmarks and exit.\n");
    printf(ARG_IDEN_HELP": displays this help and exit.\n");
    printf("###########################\n");
}

extern void argparser_free(cmdArgs_t *args) {
	if (args->benchlistStrings != NULL) {
		args->benchlistStrings = arraylist_deleteFull(args->benchlistStrings, arraylist_freeElementDefault);
	}
}
