/**
* @file gbs_ubench_atomic.c
* @brief Benchmark to evaluate the atomic functions
*		 gaspi_atomic_fetch_add and
* 		 gaspi_atomic_compare_swap.
* @author MSQC
* @version 2020-11-26
*/

#include "gbs_ubench_headers.h"
#include <assert.h>
#include <stopwatch.h>
#include <utility.h>
#include "success_or_die.h"
#include "gbs_benchmark_utils.h"

typedef void (*atomicBenchFunc) (gbs_ubench_data_t *);

/**
* Trick to convert function pointers to void pointers.
*/
typedef union ptrconv_atom_u {
	void * vptr;
	atomicBenchFunc fptr;
} ptrconv_bench_t;
static void *f2pConv(atomicBenchFunc f) {
	ptrconv_bench_t u;
	u.fptr = f;
	return u.vptr;
}
static atomicBenchFunc p2fConv(void *p) {
	ptrconv_bench_t u;
	u.vptr = p;
	return u.fptr;
}

/**
*	Previously, the segments are treated as char* and initialized as
*	Bytes. Here we use gaspi_atomic_value_t (which is an unsigned long).
*	For the benchmark the value is irrelevant, but for debugging purposes
*	it may be useful to have some meaningful data in the buffer.
*/
static void initializeSegmentValue(gaspi_segment_id_t sid) {
	gaspi_pointer_t dataV;
	gaspi_atomic_value_t *data;
	SUCCESS_OR_DIE(gaspi_segment_ptr(sid, &dataV));
	data = (gaspi_atomic_value_t*) dataV;
	*data = 0;
}

static void benchRunFetchAddSingle(gbs_ubench_data_t *benchData) {
	unsigned int i = 0;

	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		stopwatch_t time;
		initializeSegmentValue(benchData->sourceSid);
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));

		time = stopwatch_start();

		if (benchData->iproc == 0) {
			gaspi_atomic_value_t oldVal;
			SUCCESS_OR_DIE(gaspi_atomic_fetch_add(
				benchData->sourceSid, /* SID where the value is */
				0, /* Offset in the Segment */
				1, /* Target Rank */
				1, /* Value to add */
				&oldVal, /* Pointer to old value before add */
				GASPI_BLOCK /* Timeout */
			));
		}

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

static void benchRunFetchAddAll(gbs_ubench_data_t *benchData) {
	unsigned int i = 0;

	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		stopwatch_t time;
		initializeSegmentValue(benchData->sourceSid);
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));

		time = stopwatch_start();

		/* All other processes increment the first data symbol at rank 0 */
		if (benchData->iproc != 0) {
			gaspi_atomic_value_t oldVal;
			SUCCESS_OR_DIE(gaspi_atomic_fetch_add(
				benchData->sourceSid, /* SID where the value is */
				0, /* Offset in the Segment */
				0, /* Target Rank */
				1, /* Value to add */
				&oldVal, /* Pointer to old value before add */
				GASPI_BLOCK /* Timeout */
			));
		}
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

static void benchRunCasSingle(gbs_ubench_data_t *benchData) {
	unsigned int i = 0;

	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		stopwatch_t time;
		initializeSegmentValue(benchData->sourceSid);
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));

		time = stopwatch_start();

		if (benchData->iproc == 0) {
			gaspi_atomic_value_t oldVal;
			SUCCESS_OR_DIE(gaspi_atomic_compare_swap(
				benchData->sourceSid, /* SID where the value is */
				0, /* Offset in the Segment */
				1, /* Target Rank */
				0, /* Comparator value */
				1, /* New value */
				&oldVal, /* Pointer to old value before add */
				GASPI_BLOCK /* Timeout */
			));
		}
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


static void benchRunCasAll(gbs_ubench_data_t *benchData) {
	unsigned int i = 0;
	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		stopwatch_t time;
		initializeSegmentValue(benchData->sourceSid);
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));

		time = stopwatch_start();

		/* All other processes CAS the first data symbol at rank 0 */
		if (benchData->iproc != 0) {
			gaspi_atomic_value_t oldVal;
			SUCCESS_OR_DIE(gaspi_atomic_compare_swap(
				benchData->sourceSid, /* SID where the value is */
				0, /* Offset in the Segment */
				0, /* Target Rank */
				0, /* Comparator value */
				1, /* New value */
				&oldVal, /* Pointer to old value before add */
				GASPI_BLOCK /* Timeout */
			));
		}
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


static void benchRun(gbs_ubench_data_t *benchData) {
	atomicBenchFunc benchFunc = p2fConv(benchData->customData);
	benchFunc(benchData);
}

static void runMode(gbs_ubench_data_t *benchData, gbs_ubench_bench_info_t *benchInfo, gbs_ubench_execution_mode_t exMode) {
	if (exMode == UBENCH_EXMODE_INIT) {
		assert (benchInfo != NULL);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_REPETITIONS);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_TIME_AVG);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_TIME_MIN);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_TIME_MAX);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_OPS_PER_SEC);
		benchInfo->noByteDependence = true;
		benchInfo->minSegmentSize = sizeof(gaspi_atomic_value_t);
	} else if (exMode == UBENCH_EXMODE_BENCH) {
		assert (benchData != NULL);
		benchRun(benchData);
	}
}

void gbs_ubench_atomic_fetch_add_single(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, f2pConv(benchRunFetchAddSingle));
}

void gbs_ubench_atomic_fetch_add_all(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, f2pConv(benchRunFetchAddAll));
}
void gbs_ubench_atomic_compare_swap_single(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, f2pConv(benchRunCasSingle));
}
void gbs_ubench_atomic_compare_swap_all(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, f2pConv(benchRunCasAll));
}

