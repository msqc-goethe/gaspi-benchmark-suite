/**
* @file gbs_ubench_allreduce.c
* @brief Benchmark to evaluate the gaspi_allreduce function with
*		 different operations.
* @author MSQC
* @version 2020-11-26
*/

#include "gbs_ubench_headers.h"
#include <assert.h>
#include <stopwatch.h>
#include <utility.h>
#include "success_or_die.h"
#include "gbs_benchmark_utils.h"


static void benchRun(gbs_ubench_data_t *benchData) {
	unsigned int i = 0;

	gaspi_pointer_t sendPtr;
	gaspi_pointer_t rcvPtr;
	gaspi_operation_t op;

	SUCCESS_OR_DIE(gaspi_segment_ptr(benchData->sourceSid, &sendPtr));
	SUCCESS_OR_DIE(gaspi_segment_ptr(benchData->destSid, &rcvPtr));
	op = ((gaspi_operation_t) ptrToInt(benchData->customData));
	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		stopwatch_t time;
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));

		time = stopwatch_start();

		SUCCESS_OR_DIE(gaspi_allreduce(
			sendPtr, /* Pointer to local data */
			rcvPtr, /* Pointer to the data for the result */
			ceilDiv(benchData->dataSize, sizeof(int)), /* Number of elements. Must be at least an int ;( */
			op, /* the operation to perform */
			GASPI_TYPE_INT, /* The data type */
			GASPI_GROUP_ALL, /* The group */
			GASPI_BLOCK /* Timeout */
		));

		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));

		time = stopwatch_stop(time);
		if (i >= benchData->warmupIterations) {
			gbs_microbenchmark_result_add(&(benchData->benchResult), stopwatch_getUsecs(time));
		}
	}
	/* Bytes / usec = MB/s. */
	gbs_microbenchmark_result_finish(benchData, 0);
}

static void runMode(gbs_ubench_data_t *benchData, gbs_ubench_bench_info_t *benchInfo, gbs_ubench_execution_mode_t exMode) {
	if (exMode == UBENCH_EXMODE_INIT) {
		gaspi_number_t maxTransferSize;
		assert (benchInfo != NULL);
		SUCCESS_OR_DIE(gaspi_allreduce_elem_max(
			&maxTransferSize
		));
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_BYTES);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_REPETITIONS);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_TIME_AVG);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_TIME_MIN);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_TIME_MAX);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_OPS_PER_SEC);
		benchInfo->maxSize = (maxTransferSize * sizeof(int));
		benchInfo->minSize = sizeof(int);
		benchInfo->minSegmentSize = sizeof(int);
	} else if (exMode == UBENCH_EXMODE_BENCH) {
		assert (benchData != NULL);
		benchRun(benchData);
	}
}

void gbs_ubench_allreduce_min(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, intToPtr(GASPI_OP_MIN));
}

void gbs_ubench_allreduce_max(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, intToPtr(GASPI_OP_MAX));
}

void gbs_ubench_allreduce_sum(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, intToPtr(GASPI_OP_SUM));
}
