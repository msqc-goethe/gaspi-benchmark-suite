/**
* @file gbs_ubench_twosided_ping_pong.c
* @brief Benchmark to evaluate the two-sided
*		 communication routines gaspi_passive_send
*		 and gaspi_passive_receive.
*		 Communication happens between pairs of two ranks.
* @author Florian Beenen
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
		assert (((benchData->nproc % 2) == 0) && "Number of processes must be even for ping-pong test.");
		time = stopwatch_start();
		if ((benchData->iproc % 2) == 0) {
			gaspi_rank_t rcvRank;
			/* Send the data */
			SUCCESS_OR_DIE(gaspi_passive_send(
				benchData->sourceSid,
				0, /* L-offset */
				(benchData->iproc + 1), /* Target Rank. This rank is even, so no overflow. */
				benchData->dataSize, /* Size */
				GASPI_BLOCK
			));

			/* Receive the 'pong' */
			SUCCESS_OR_DIE(gaspi_passive_receive(
				benchData->destSid, /* SID*/
				0, /* Local offset */
				&rcvRank, /* Rank of the sender */
				benchData->dataSize,
				GASPI_BLOCK
			));
			assert (rcvRank == (benchData->iproc + 1));
		} else {
			gaspi_rank_t rcvRank;
			/* Receive the data */
			SUCCESS_OR_DIE(gaspi_passive_receive(
				benchData->destSid, /* SID*/
				0, /* Local offset */
				&rcvRank, /* Rank of the sender */
				benchData->dataSize,
				GASPI_BLOCK
			));
			assert (rcvRank == (benchData->iproc - 1));
			/* Send the data back */
			SUCCESS_OR_DIE(gaspi_passive_send(
				benchData->destSid,
				0, /* L-offset */
				rcvRank, /* Target Rank. This rank is even, so no overflow. */
				benchData->dataSize, /* Size */
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
	/* Bytes / usec = MB/s. Factor 2: Transfer was back and forth. */
	gbs_microbenchmark_result_finish(benchData, 2);
}

static void runMode(gbs_ubench_data_t *benchData, gbs_ubench_bench_info_t *benchInfo, gbs_ubench_execution_mode_t exMode) {
	if (exMode == UBENCH_EXMODE_INIT) {
		gaspi_size_t maxTransferSize;
		assert (benchInfo != NULL);

		SUCCESS_OR_DIE(gaspi_passive_transfer_size_max(
			&maxTransferSize
		));
		benchInfo->maxSize = maxTransferSize;
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

void gbs_ubench_twosided_ping_pong(gbs_bench_config_t *conf) {
	gbs_microbenchmark_execute(conf, runMode);
}