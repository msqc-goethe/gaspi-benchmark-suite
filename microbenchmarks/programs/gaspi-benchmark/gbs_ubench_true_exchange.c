/**
* @file gbs_ubench_true_exchange.c
* @brief Benchmark to evaluate the true one-sided
*		 commuication: First, the unidirectional transfer latency
*		 for put or get is measured. Afterwards, the transfer is
*		 repeated while the remote side performs CPU-intensive work.
* @author Florian Beenen
* @version 2020-11-27
*/

#include "gbs_ubench_headers.h"
#include <assert.h>
#include <string.h>
#include <stopwatch.h>
#include "success_or_die.h"
#include "gbs_benchmark_utils.h"

/**
* Differentiate between GET und PUT based
* transfer modes in the multiplex function.
*/
typedef enum true_exchange_bench_e {
	MODE_GET,
	MODE_PUT
} true_exchange_bench_t;

static void benchRunPut(gbs_ubench_data_t *benchData) {
	unsigned int i;
	gbs_ubench_data_t prerunData;
	double transferUsecs = 0;
	memcpy(&prerunData, benchData, sizeof(*benchData));
	gbs_ubench_put_single_udir_benchRun(&prerunData);
	transferUsecs = prerunData.benchResult.measuredTimeAvg;

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
			gbs_utils_loadCpu(transferUsecs);
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

static void benchRunGet(gbs_ubench_data_t *benchData) {
	unsigned int i = 0;
	gbs_ubench_data_t prerunData;
	double transferUsecs = 0;
	memcpy(&prerunData, benchData, sizeof(*benchData));
	gbs_ubench_get_single_udir_benchRun(&prerunData);
	transferUsecs = prerunData.benchResult.measuredTimeAvg;

	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		gaspi_notification_id_t nid;
		gaspi_notification_t noti = 0;
		stopwatch_t time;
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));
		time = stopwatch_start();
		if (benchData->iproc == 0) {
			SUCCESS_OR_DIE(gaspi_read_notify(
				benchData->destSid, /* Local SID */
				0, /* Local Offset */
				1, /* Target Rank */
				benchData->sourceSid, /* Remote SID */
				0, /* Remote Offset */
				benchData->dataSize, /* Data size */
				1, /* Noti-ID */
				0, /* Queue */
				GASPI_BLOCK /* Timeout */
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
			SUCCESS_OR_DIE(gaspi_wait( /* Flush the Queue */
				0, /* Queue */
				GASPI_BLOCK
			));
		} else if (benchData->iproc == 1) {
			gbs_utils_loadCpu(transferUsecs);
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
		true_exchange_bench_t mode = (true_exchange_bench_t) ptrToInt(benchData->customData);
		switch (mode) {
			case MODE_GET:
				benchRunGet(benchData);
			break;
			case MODE_PUT:
				benchRunPut(benchData);
			break;
			default:
			assert(false);
			break;
		}
		assert (benchData != NULL);
	}
}

void gbs_ubench_put_true_exchange(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, intToPtr(MODE_PUT));
}
void gbs_ubench_get_true_exchange(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, intToPtr(MODE_GET));
}
