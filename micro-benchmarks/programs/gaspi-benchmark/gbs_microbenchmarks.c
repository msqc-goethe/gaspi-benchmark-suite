/**
* @file gbs_microbenchmarks.c
* @brief Module that executes all microbenchmarks.
* @detail This file contains functions that are used by all
*	      microbenchmarks.
* @author Florian Beenen
* @version 2020-11-26
*/

#include "gbs_microbenchmarks.h"
#include <string.h>
#include <GASPI.h>
#include <assert.h>
#include "success_or_die.h"
#include "gbs_benchmark_utils.h"

/**
* Writes the benchmark results to the output stream. The numbers are separated
* by whitespace and padded so that the numbers align with the column headings.
*
* @param f The output stream.
* @param colStartList List with integers (!) in which the column start positions
*		 are encoded.
* @param benchData The benchmark data that contains the result.
* @param outputMode Bitset of metrics to be printed.
*/
static void printResult(FILE *f, arraylist_t *colStartList, gbs_ubench_data_t *benchData, gbs_ubench_output_mode_t outputMode) {
	unsigned int col = 0;
	unsigned int listIdx = 0;
	gbs_ubench_result_t *benchResult = &(benchData->benchResult);
	if (gbs_ubench_testBenchMode(outputMode, UBENCH_OUTMODE_BYTES)) {
		unsigned int i = ptrToInt(arraylist_get(colStartList, listIdx++));
		gbs_util_fillStreamToCol(f, &col, i);
		col += fprintf(f, "%d", benchData->dataSize);
	}
	if (gbs_ubench_testBenchMode(outputMode, UBENCH_OUTMODE_REPETITIONS)) {
		unsigned int i = ptrToInt(arraylist_get(colStartList, listIdx++));
		gbs_util_fillStreamToCol(f, &col, i);
		col += fprintf(f, "%d", benchData->measureIterations);
	}
	if (gbs_ubench_testBenchMode(outputMode, UBENCH_OUTMODE_TIME_MIN)) {
		unsigned int i = ptrToInt(arraylist_get(colStartList, listIdx++));
		gbs_util_fillStreamToCol(f, &col, i);
		col += fprintf(f, "%"DECIMAL_PRECISION, benchResult->measuredTimeMin);
	}
	if (gbs_ubench_testBenchMode(outputMode, UBENCH_OUTMODE_TIME_MAX)) {
		unsigned int i = ptrToInt(arraylist_get(colStartList, listIdx++));
		gbs_util_fillStreamToCol(f, &col, i);
		col += fprintf(f, "%"DECIMAL_PRECISION, benchResult->measuredTimeMax);
	}
	if (gbs_ubench_testBenchMode(outputMode, UBENCH_OUTMODE_TIME_AVG)) {
		unsigned int i = ptrToInt(arraylist_get(colStartList, listIdx++));
		gbs_util_fillStreamToCol(f, &col, i);
		col += fprintf(f, "%"DECIMAL_PRECISION, benchResult->measuredTimeAvg);
	}
	if (gbs_ubench_testBenchMode(outputMode, UBENCH_OUTMODE_OPS_PER_SEC)) {
		unsigned int i = ptrToInt(arraylist_get(colStartList, listIdx++));
		gbs_util_fillStreamToCol(f, &col, i);
		col += fprintf(f, "%"DECIMAL_PRECISION, benchResult->opsPerSec);
	}
	if (gbs_ubench_testBenchMode(outputMode, UBENCH_OUTMODE_BW)) {
		unsigned int i = ptrToInt(arraylist_get(colStartList, listIdx++));
		gbs_util_fillStreamToCol(f, &col, i);
		col += fprintf(f, "%"DECIMAL_PRECISION, benchResult->measuredBandwidth);
	}
	fprintf(f, "\n");
	fflush(f);
}

/**
* Write the column heading line to the given stream.
* To align the results written later properly, this function
* generates a list with the columns in the stream where the
* respective column heading is written.
*
* @param f The output stream.
* @param outputMode Bitset of metrics to be printed.
* @return Allocated list with integers (!) inside (misused as pointers).
*		  Numbers indicate the staring column of a column heading.
*/
static arraylist_t *printHeading(FILE *f, gbs_ubench_output_mode_t outputMode) {
	arraylist_t *colStartList = arraylist_create();
	arraylist_t *headings = arraylist_create();
	const unsigned int separationSpaces = OUTPUT_COLUMN_SPACING;
	unsigned int i;
	unsigned int fullLength = 1;
	unsigned int currentOffset = 0;
	char *buffer = NULL;
	char spaces[separationSpaces + 1];
	/* Attention: The order here must match the enum gbs_ubench_output_mode_t */
	arraylist_add(headings, "#bytes");
	arraylist_add(headings, "#repetitions");
	arraylist_add(headings, "t_min[usec]");
	arraylist_add(headings, "t_max[usec]");
	arraylist_add(headings, "t_avg[usec]");
	arraylist_add(headings, "Ops[1/sec]");
	arraylist_add(headings, "Mbytes/sec");

	for (i = 0; i < arraylist_getLength(headings); ++i) {
		char *str = arraylist_get(headings, i);
		fullLength += strlen(str);
		fullLength += separationSpaces;
	}

	buffer = safeMalloc(sizeof(char) * fullLength);
	memset(spaces, ' ', separationSpaces);
	spaces[separationSpaces] = '\0';

	for (i = 0; i < arraylist_getLength(headings); ++i) {
		if (gbs_ubench_testBenchMode(outputMode, i + 1)) {
			arraylist_add(colStartList, intToPtr(currentOffset + separationSpaces));
			currentOffset += sprintf(buffer + currentOffset, "%s%s", spaces, (char*) arraylist_get(headings, i));
		}
	}

	fprintf(f, "%s\n", buffer);
	fflush(f);
	free(buffer);
	headings = arraylist_delete(headings);
	return colStartList;
}

/**
* Calculate the number of measurement repetitions with respect to the
* benchmark configuration from the command-line arguments.
* Depending on the transfer size, less repetitions are performed.
*
* @param args The parsed command-line arguments.
* @param currentTransferSize The chunk size to be transferred.
* @return Number of repetitions for the benchmark (w/o warm-up).
*/
static unsigned int getMeasurementIterations(cmdArgs_t *args, unsigned int currentTransferSize) {
	unsigned int reduceThres = args->runReduceThres;
	unsigned int iterations;
	if ((reduceThres == 0) || (currentTransferSize < (reduceThres))) {
		return args->maxRuns;
	}
	reduceThres = ceilDiv(reduceThres, args->sizeFactor) * (args->sizeFactor); /* Round thres up to align with the factor */
	iterations = (args->maxRuns / ( (currentTransferSize * args->sizeFactor) / reduceThres));
	iterations = max(iterations, args->minRuns);
	return min(iterations, args->maxRuns);
}

void gbs_microbenchmark_execute(gbs_bench_config_t *conf, gbs_microbenchmark_func benchFunc) {
	gbs_microbenchmark_execute_custom(conf, benchFunc, NULL);
}

void gbs_microbenchmark_execute_custom(gbs_bench_config_t *conf, gbs_microbenchmark_func benchFunc, void *customData) {
	unsigned int maximumTransferSize = ceilDiv(conf->cmdArgs->sizeEnd, conf->cmdArgs->sizeFactor) * conf->cmdArgs->sizeFactor;
	unsigned int minimumTransferSize = conf->cmdArgs->sizeStart;
	unsigned int currentTransferSize = 0;
	arraylist_t *colStartList = NULL;
	gaspi_segment_id_t transferSourceSid = 0;
	gaspi_segment_id_t transferDestSid = 1;
	gaspi_segment_id_t resultsSid = 2;
	gbs_ubench_bench_info_t benchInfo;
	gbs_ubench_data_t benchData;

	memset(&benchInfo, 0, sizeof(benchInfo)); /* initialize */
	benchFunc(NULL, &benchInfo, UBENCH_EXMODE_INIT);
	if (benchInfo.maxSize > 0) {
		maximumTransferSize = min(maximumTransferSize, benchInfo.maxSize);
	}
	if (benchInfo.minSize > 0) {
		minimumTransferSize = max(minimumTransferSize, benchInfo.minSize);
	}

	/* Initialize the segments */
	SUCCESS_OR_DIE(gaspi_segment_create(
		transferSourceSid, /* SID */
		max(maximumTransferSize * (conf->nproc), benchInfo.minSegmentSize), /* Size */
		GASPI_GROUP_ALL, /* Group */
		GASPI_BLOCK, /* Timeout */
		GASPI_MEM_UNINITIALIZED /* Init */
	));
	SUCCESS_OR_DIE(gaspi_segment_create(
		transferDestSid, /* SID */
		max(maximumTransferSize * (conf->nproc), benchInfo.minSegmentSize), /* Size */
		GASPI_GROUP_ALL, /* Group */
		GASPI_BLOCK, /* Timeout */
		GASPI_MEM_UNINITIALIZED /* Init */
	));
	SUCCESS_OR_DIE(gaspi_segment_create(
		resultsSid, /* SID */
		sizeof(benchData.benchResult) * (conf->nproc), /* Size */
		GASPI_GROUP_ALL, /* Group */
		GASPI_BLOCK, /* Timeout */
		GASPI_MEM_UNINITIALIZED /* Init */
	));
	gbs_util_initSegment(transferSourceSid, maximumTransferSize * (conf->nproc));

	if (conf->iproc == 0) {
		colStartList = printHeading(conf->outputStream, benchInfo.outputMode);
	}

	benchData.sourceSid = transferSourceSid;
	benchData.destSid = transferDestSid;
	benchData.resultsSid = resultsSid;
	benchData.timeCombined = conf->cmdArgs->timeCombined;
	benchData.aggregateCount = conf->cmdArgs->aggregateCount;
	benchData.iproc = conf->iproc;
	benchData.nproc = conf->nproc;
	benchData.customData = customData;

	/* If no byte depencence is there, run the benchmark
	   exactly once
	 */
	currentTransferSize = minimumTransferSize;
	/* Run the Benchmark */
	if (benchInfo.noByteDependence) {
		memset(&(benchData.benchResult), 0, sizeof(benchData.benchResult));
		benchData.measureIterations = conf->cmdArgs->maxRuns;
		benchData.warmupIterations = ((conf->cmdArgs->warmupPercent) * benchData.measureIterations) / 100;
		benchData.dataSize = 0;
		benchFunc(&benchData, NULL, UBENCH_EXMODE_BENCH); /* Perform the benchmark */
		if (conf->iproc == 0) {
			printResult(conf->outputStream, colStartList, &benchData, benchInfo.outputMode);
		}
	} else {
		while (currentTransferSize <= maximumTransferSize) {
			memset(&(benchData.benchResult), 0, sizeof(benchData.benchResult));

			benchData.measureIterations = getMeasurementIterations(conf->cmdArgs, currentTransferSize);
			benchData.warmupIterations = ((conf->cmdArgs->warmupPercent) * benchData.measureIterations) / 100;
			benchData.dataSize = currentTransferSize;
			benchFunc(&benchData, NULL, UBENCH_EXMODE_BENCH); /* Perform the benchmark */

			if (conf->iproc == 0) {
				printResult(conf->outputStream, colStartList, &benchData, benchInfo.outputMode);
			}

			/* If the last iteration exceeds MTS, run the last iteration exactly with the MTS. */
			if ((currentTransferSize < maximumTransferSize) && ((currentTransferSize * conf->cmdArgs->sizeFactor) > maximumTransferSize))  {
				currentTransferSize = maximumTransferSize;
			} else {
				currentTransferSize *= conf->cmdArgs->sizeFactor;
			}
		}
	}

	benchFunc(&benchData, NULL, UBENCH_EXMODE_FINISH); /* Perform potential cleanup */

	colStartList = arraylist_delete(colStartList);

	SUCCESS_OR_DIE(gaspi_segment_delete(
		transferSourceSid
	));
	SUCCESS_OR_DIE(gaspi_segment_delete(
		transferDestSid
	));
	SUCCESS_OR_DIE(gaspi_segment_delete(
		resultsSid
	));
}


void gbs_microbenchmark_result_add(gbs_ubench_result_t *result, double usecs) {
	assert(result != NULL);
	if (result->measuredTimeAvg == 0.0) { /* First run */
		result->measuredTimeMin = usecs; /* Initialize minimum */
	}
	result->measuredTimeAvg += usecs;
	result->measuredTimeMin = min(result->measuredTimeMin, usecs);
	result->measuredTimeMax = max(result->measuredTimeMax, usecs);
}

void gbs_microbenchmark_result_finish(gbs_ubench_data_t *benchData, unsigned int bandwidthMultiplier) {
	assert(benchData != NULL);
	benchData->benchResult.measuredTimeAvg /= benchData->measureIterations;
	benchData->benchResult.opsPerSec = (1e6 / benchData->benchResult.measuredTimeAvg);
	benchData->benchResult.measuredBandwidth = (benchData->dataSize / (benchData->benchResult.measuredTimeAvg)) * bandwidthMultiplier;

	if (benchData->timeCombined) { /* Fetch data from all Ranks */
		gbs_ubench_result_t *allResults;
		void *allResultsV;
		SUCCESS_OR_DIE(gaspi_segment_ptr(benchData->resultsSid, &allResultsV));
		allResults = (gbs_ubench_result_t *) allResultsV;
		memcpy(allResults, &(benchData->benchResult), sizeof(benchData->benchResult));
		gbs_utils_gather0(benchData->resultsSid, sizeof(*allResults));

		if (benchData->iproc == 0) {
			unsigned int proc;
			unsigned int validProcs = 1;
			/* Aggregate process data */
			for (proc = 1; proc < benchData->nproc; ++proc) {
				gbs_ubench_result_t *otherResult = &(allResults[proc]);
				if (!(otherResult->valuesInvalid)) {
					benchData->benchResult.measuredTimeMin += otherResult->measuredTimeMin;
					benchData->benchResult.measuredTimeMax += otherResult->measuredTimeMax;
					benchData->benchResult.measuredTimeAvg += otherResult->measuredTimeAvg;
					benchData->benchResult.opsPerSec += otherResult->opsPerSec;
					benchData->benchResult.measuredBandwidth += otherResult->measuredBandwidth;
					++validProcs;
				}
			}
			benchData->benchResult.measuredTimeMin /= validProcs;
			benchData->benchResult.measuredTimeMax /= validProcs;
			benchData->benchResult.measuredTimeAvg /= validProcs;
			benchData->benchResult.opsPerSec /= validProcs;
			benchData->benchResult.measuredBandwidth /= validProcs;
		}
	}
}
