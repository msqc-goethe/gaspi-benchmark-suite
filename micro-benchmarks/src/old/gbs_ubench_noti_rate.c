/**
* @file gbs_ubench_noti_rate.c
* @brief Benchmark to evaluate the notification
*		 rate when using the gaspi_notify
*		 function.
* @author MSQC
* @version 2020-11-26
*/

#include "gbs_ubench_headers.h"
#include <assert.h>
#include <stopwatch.h>
#include <utility.h>
#include "success_or_die.h"
#include "gbs_benchmark_utils.h"

/**
* Enum to differentiate between different measurements.
* SINGLE_UDIR : Send a notification from rank 0 to rank 1.
* SINGLE_BDIR : Rank 0 and Rank 1 exchange one notifications
*			    in a concurrent way.
* ALL_UDIR : The first rank sends one notification to every other rank
*			 but to itself.
* ALL_BDIR : Every rank sends a notification to every other rank
*			 but to itself.
*/
typedef enum noti_rate_bench_e {
	SINGLE_UDIR,
	SINGLE_BDIR,
	ALL_UDIR,
	ALL_BDIR
} noti_rate_bench_t;

static void benchRunSingleUdir(gbs_ubench_data_t *benchData) {
	unsigned int i;

	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		stopwatch_t time;
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));
		time = stopwatch_start();

		if (benchData->iproc == 0) {
			SUCCESS_OR_DIE(gaspi_notify(
				benchData->destSid, /* SID */
				1, /* Target Rank */
				benchData->iproc, /* Notification ID */
				1, /* Notification value */
				0, /* Queue */
				GASPI_BLOCK /* Timeout */
			));
			/* Flush the Queue */
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

	gbs_microbenchmark_result_finish(benchData, 0);
}

static void benchRunSingleBdir(gbs_ubench_data_t *benchData) {
	unsigned int i;

	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		stopwatch_t time;
		gaspi_notification_id_t nid;
		gaspi_notification_t noti = 0;

		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));
		time = stopwatch_start();

		if (benchData->iproc < 2) {
			SUCCESS_OR_DIE(gaspi_notify(
				benchData->destSid, /* SID */
				(benchData->iproc + 1) % 2, /* Target Rank */
				benchData->iproc, /* Notification ID */
				1, /* Notification value */
				0, /* Queue */
				GASPI_BLOCK /* Timeout */
			));
			/* Flush the Queue */
			SUCCESS_OR_DIE(gaspi_wait( /* Flush the Queue */
				0, /* Queue */
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

	gbs_microbenchmark_result_finish(benchData, 0);
}

static void benchRunAllUdir(gbs_ubench_data_t *benchData) {
	unsigned int i;

	for (i = 0; i < (benchData->warmupIterations + benchData->measureIterations); ++i) {
		stopwatch_t time;
		SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
		));
		time = stopwatch_start();

		if (benchData->iproc == 0) {
			unsigned int proc;
			gaspi_queue_id_t currentQueue = 0;
			gaspi_queue_id_t maxUsedQueue = 0;
			for (proc = 1; proc < benchData->nproc; ++proc) {
				currentQueue = gbs_utils_getNextFreeQueue(currentQueue);
				maxUsedQueue = max(maxUsedQueue, currentQueue);
				SUCCESS_OR_DIE(gaspi_notify(
					benchData->destSid, /* SID */
					proc, /* Target Rank */
					benchData->iproc, /* Notification ID */
					1, /* Notification value */
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
		} else {
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

	gbs_microbenchmark_result_finish(benchData, 0);
}

static void benchRunAllBdir(gbs_ubench_data_t *benchData) {
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

		for (proc = 1; proc < benchData->nproc; ++proc) {
			gaspi_rank_t actualProc = (benchData->iproc + proc) % (benchData->nproc);
			currentQueue = gbs_utils_getNextFreeQueue(currentQueue);
			maxUsedQueue = max(maxUsedQueue, currentQueue);
			SUCCESS_OR_DIE(gaspi_notify(
				benchData->destSid, /* SID */
				actualProc, /* Target Rank */
				benchData->iproc, /* Notification ID */
				1, /* Notification value */
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

		/* Receive Notifications */
		for (proc = 1; proc < benchData->nproc; ++proc) {
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

	gbs_microbenchmark_result_finish(benchData, 0);
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
	} else if (exMode == UBENCH_EXMODE_BENCH) {
		assert (benchData != NULL);
		noti_rate_bench_t benchType = (noti_rate_bench_t) ptrToInt(benchData->customData);
		switch (benchType) {
			case SINGLE_UDIR:
				benchRunSingleUdir(benchData);
				break;
			case SINGLE_BDIR:
				benchRunSingleBdir(benchData);
				break;
			case ALL_UDIR:
				benchRunAllUdir(benchData);
				break;
			case ALL_BDIR:
				benchRunAllBdir(benchData);
				break;
			default:
				assert(false);
				break;
		}
	}
}

void gbs_ubench_noti_single_udir(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, intToPtr(SINGLE_UDIR));
}
void gbs_ubench_noti_single_bdir(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, intToPtr(SINGLE_BDIR));
}
void gbs_ubench_noti_all_udir(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, intToPtr(ALL_UDIR));
}
void gbs_ubench_noti_all_bdir(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute_custom(conf, runMode, intToPtr(ALL_BDIR));
}
