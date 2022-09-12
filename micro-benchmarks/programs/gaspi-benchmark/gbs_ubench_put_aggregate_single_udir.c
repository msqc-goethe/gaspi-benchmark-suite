/**
* @file gbs_ubench_put_aggregate_single_udir.c
* @brief Benchmark to evaluate unidirectional put
*		 performance between two nodes with aggregated
*		 transfer. This benchmark aggregates multiple
*		 gaspi_write transfers and sends a notification only
*		 after all RMA request have been scheduled.
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
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));
		time = stopwatch_start();
		if (benchData->iproc == 0) {
			unsigned int aggregate;
			for (aggregate = 0; aggregate < benchData->aggregateCount; ++aggregate) {
				SUCCESS_OR_DIE(gaspi_write(
					benchData->sourceSid,
					0, /* L-offset */
					1, /* Target Rank */
					benchData->destSid, /* Remote SID */
					0,	/* R-offset */
					benchData->dataSize, /* Size */
					0, /* Queue	 */
					GASPI_BLOCK
				));
			}
			SUCCESS_OR_DIE(gaspi_notify(
				benchData->destSid, /* SID */
				1, /* Target Rank */
				benchData->iproc, /* Notification ID */
				1, /* Notification value */
				0, /* Queue */
				GASPI_BLOCK /* Timeout */
			));

			/* Flush the Queues */
			SUCCESS_OR_DIE(gaspi_wait( /* Flush the Queue */
				0, /* Queue */
				GASPI_BLOCK
			));
		} else if (benchData->iproc == 1) {
			gaspi_notification_id_t nid;
			gaspi_notification_t noti = 0;
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
			assert(noti == 1);
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
	/* Bytes / usec = MB/s.   */
	gbs_microbenchmark_result_finish(benchData, benchData->aggregateCount);
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

void gbs_ubench_put_aggregate_single_udir(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute(conf, runMode);
}
