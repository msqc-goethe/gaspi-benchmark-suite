/**
* @file gbs_ubench_put_single_udir.c
* @brief Benchmark to evaluate unidirectional
*		 put RMA transfer that involves only the first two ranks.
*		 The first rank sends a block of data to the second rank.
* @author Florian Beenen
* @version 2020-11-27
*/

#include "gbs_ubench_headers.h"
#include <assert.h>
#include <stopwatch.h>
#include "success_or_die.h"
#include "gbs_benchmark_utils.h"

void gbs_ubench_put_single_udir_benchRun(gbs_ubench_data_t *benchData) {
	unsigned int i;

	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		stopwatch_t time;
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));
		time = stopwatch_start();
		if (benchData->iproc == 0) {
			SUCCESS_OR_DIE(gaspi_write_notify(
				benchData->sourceSid,
				0, /* L-offset */
				1, /* Target Rank */
				benchData->destSid, /* Remote SID */
				0,	/* R-offset */
				benchData->dataSize, /* Size */
				benchData->iproc, /* Noti-ID */
				1, /* Noti-value */
				0, /* Queue	 */
				GASPI_BLOCK
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
	gbs_microbenchmark_result_finish(benchData, 1);
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
		gbs_ubench_put_single_udir_benchRun(benchData);
	}
}

void gbs_ubench_put_single_udir(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute(conf, runMode);
}