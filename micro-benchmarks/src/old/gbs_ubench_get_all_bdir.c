/**
* @file gbs_ubench_get_all_bdir.c
* @brief Benchmark to evaluate the bidirectional
*		 get RMA transfer function involving all nodes.
* @author MSQC
* @version 2020-11-26
*/

#include "gbs_ubench_headers.h"
#include <assert.h>
#include <stopwatch.h>
#include "success_or_die.h"
#include "gbs_benchmark_utils.h"

static void benchRun(gbs_ubench_data_t *benchData) {
	unsigned int i;

	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		stopwatch_t time;
		unsigned int proc;
		gaspi_queue_id_t currentQueue = 0;
		gaspi_queue_id_t maxUsedQueue = 0;
		gaspi_notification_id_t nid;
		gaspi_notification_t noti = 0;
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));
		time = stopwatch_start();

		/* Post all the read requests */
		for (proc = 1; proc < benchData->nproc; ++proc) {
			/* Avoid congestions by sending to the next ranks first */
			unsigned int actualProc = (benchData->iproc + proc) % (benchData->nproc);
			currentQueue = gbs_utils_getNextFreeQueue(currentQueue);
			maxUsedQueue = max(maxUsedQueue, currentQueue);
			SUCCESS_OR_DIE(gaspi_read_notify(
				benchData->destSid, /* Local SID */
				(benchData->dataSize) * (actualProc), /* Local Offset. Every read to a different location */
				actualProc, /* Target Rank */
				benchData->sourceSid, /* Remote SID */
				0, /* Remote Offset */
				benchData->dataSize, /* Data size */
				actualProc, /* Noti-ID */
				currentQueue, /* Queue */
				GASPI_BLOCK /* Timeout */
			));
		}
		/* Flush all used Queues */
		for (currentQueue = 0; currentQueue <= maxUsedQueue; ++currentQueue) {
			SUCCESS_OR_DIE(gaspi_wait( /* Flush the Queue */
				currentQueue, /* Queue */
				GASPI_BLOCK
			));
		}
		/* Collect the notifications */
		for (proc = 1; proc < benchData->nproc; ++proc) {
			SUCCESS_OR_DIE(gaspi_notify_waitsome(
				benchData->destSid, /* SID */
				0, /* Start NID */
				benchData->nproc, /* Number of notis */
				&nid, /* The received notification-ID */
				GASPI_BLOCK
			));
			SUCCESS_OR_DIE(gaspi_notify_reset(
				benchData->destSid, /* SID */
				nid, /* NID to reset */
				&noti /* Old noti value */
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
	/* Bytes / usec = MB/s.  In every cycle, data was transferred to all nodes except rank 0. */
	gbs_microbenchmark_result_finish(benchData, benchData->nproc - 1);
}


static void runMode(gbs_ubench_data_t *benchData, gbs_ubench_bench_info_t *benchInfo, gbs_ubench_execution_mode_t exMode) {
	if (exMode == UBENCH_EXMODE_INIT) {
		assert (benchInfo != NULL);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_BYTES);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_REPETITIONS);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_TIME_AVG);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_TIME_MIN);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_TIME_MAX);
		gbs_ubench_addBenchMode(&(benchInfo->outputMode), UBENCH_OUTMODE_BW);
	} else if (exMode == UBENCH_EXMODE_BENCH) {
		assert (benchData != NULL);
		benchRun(benchData);
	}
}

void gbs_ubench_get_all_bdir(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute(conf, runMode);
}
