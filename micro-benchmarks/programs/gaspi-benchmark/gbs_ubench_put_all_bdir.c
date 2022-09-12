/**
* @file gbs_ubench_put_all_bdir.c
* @brief Benchmark to evaluate bidirectional
*		 put RMA transfer that involves all ranks.
*		 All ranks send data from all other ranks but
*		 to themselves.
* @author MSQC
* @version 2020-11-27
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
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));
		time = stopwatch_start();

		/* Schedule sends to everyone */
		for (proc = 1; proc < benchData->nproc; ++proc) {
			/* Avoid congestions by sending to the next ranks first */
			unsigned int actualProc = (benchData->iproc + proc) % (benchData->nproc);
			currentQueue = gbs_utils_getNextFreeQueue(currentQueue);
			maxUsedQueue = max(maxUsedQueue, currentQueue);
			SUCCESS_OR_DIE(gaspi_write_notify(
				benchData->sourceSid,
				0, /* L-offset */
				actualProc, /* Target Rank */
				benchData->destSid, /* Remote SID */
				(benchData->dataSize) * (benchData->iproc), /* R-offset. Everyone writes to a different spot.*/
				benchData->dataSize, /* Size */
				(benchData->iproc), /* Noti-ID. Every send needs different ID to avoid overwrites */
				1, /* Noti-value */
				currentQueue, /* Queue */
				GASPI_BLOCK
			));
		}
		/* Receive notifications */
		for (proc = 1; proc < benchData->nproc; ++proc) {
			gaspi_notification_id_t nid;
			gaspi_notification_t noti = 0;
			SUCCESS_OR_DIE(gaspi_notify_waitsome(
				benchData->destSid, /* SID */
				0, /* Start NID */
				benchData->nproc, /* Number of consecutive noti-IDs to wait for*/
				&nid, /* The received notification-ID */
				GASPI_BLOCK
			));
			SUCCESS_OR_DIE(gaspi_notify_reset(
				benchData->destSid, /* SID */
				nid, /* NID to reset */
				&noti /* Old noti value */
			));
			assert(noti == 1);
		}

		/* Flush all used Queues */
		for (currentQueue = 0; currentQueue <= maxUsedQueue; ++currentQueue) {
			SUCCESS_OR_DIE(gaspi_wait( /* Flush the Queue */
				currentQueue, /* Queue */
				GASPI_BLOCK
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

void gbs_ubench_put_all_bdir(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute(conf, runMode);
}
