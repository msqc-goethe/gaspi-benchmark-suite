/**
* @file gbs_ubench_ping_pong.c
* @brief Benchmark to evaluate the ping-pong
*		 communication scheme. The gaspi_write_notify
*		 routine is used to transfer data.
* @author MSQC
* @version 2020-11-26
*/

#include "gbs_ubench_headers.h"
#include <assert.h>
#include <stopwatch.h>
#include "gbs_benchmark_utils.h"
#include "success_or_die.h"

static void benchRun(gbs_ubench_data_t *benchData) {
	unsigned int i;

	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		gaspi_notification_id_t nid;
		gaspi_notification_t noti = 0;
		stopwatch_t time;
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));
		assert (((benchData->nproc % 2) == 0) && "Number of processes must be even for ping-pong test.");
		time = stopwatch_start();
		if ((benchData->iproc % 2) == 0) {
			/* Send + Wait + Flush */
			SUCCESS_OR_DIE(gaspi_write_notify(
				benchData->sourceSid,
				0, /* L-offset */
				(benchData->iproc + 1), /* Target Rank. This rank is even, so no overflow. */
				benchData->destSid, /* Remote SID */
				0,	/* R-offset */
				benchData->dataSize, /* Size */
				benchData->iproc, /* Noti-ID */
				1, /* Noti-value */
				0, /* Queue	 */
				GASPI_BLOCK
			));
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
		} else {
			/* Wait + Send  + Flush */
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
			SUCCESS_OR_DIE(gaspi_write_notify(
				benchData->sourceSid,
				0, /* L-offset */
				benchData->iproc - 1, /* Target Rank. This rank is odd - the original rank was therefore rank-1. */
				benchData->destSid, /* Remote SID */
				0,	/* R-offset */
				benchData->dataSize, /* Size */
				benchData->iproc, /* Noti-ID */
				1, /* Noti-value */
				0, /* Queue	 */
				GASPI_BLOCK
			));
		}
		SUCCESS_OR_DIE(gaspi_wait( /* Flush the Queue */
			0, /* Queue */
			GASPI_BLOCK
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
	/* Bytes / usec = MB/s. Factor 2: Transfer was back and forth. */
	gbs_microbenchmark_result_finish(benchData, 2);
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

void gbs_ubench_ping_pong(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute(conf, runMode);
}
