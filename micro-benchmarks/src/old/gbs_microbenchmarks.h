/**
* @file gbs_microbenchmarks.h
* @brief Module that executes all microbenchmarks.
* @detail This file contains functions that are used by all
*	      microbenchmarks.
* @author MSQC
* @version 2020-11-26
*/

#ifndef __GBS_MICROBENCHMARKS__
#define __GBS_MICROBENCHMARKS__

#include <utility.h>
#include "gbs_benchmark.h"


/**
* Check if a benchmark mode of type 'gbs_ubench_output_mode_t' contains
* a given attribute of that enum.
* 
* @param mode Variable of type 'gbs_ubench_output_mode_t'
* @param toTest One of the values of 'gbs_ubench_output_mode_t'.
* @return true, if the given bit-set contains the attribute to test.
*/
#define gbs_ubench_testBenchMode(mode, toTest) ( ((1 << (toTest)) & (mode)) != 0)

/**
* Add a output mode to the given bit set.
*
* @param mode Variable of type 'gbs_ubench_output_mode_t' to which the
*		 'newMode' should be added.
* @param newMode A value of type 'gbs_ubench_output_mode_t' that is added
*		 to 'mode'.
*/
#define gbs_ubench_addBenchMode(modePtr, newMode) ( (*(modePtr)) |= (1 << newMode))

/**
* Type for specifying the output behavior of a benchmark.
* This type is designed as a bit set. It should be manipulated
* with the macros 'gbs_ubench_testBenchMode' and 'gbs_ubench_addBenchMode'.
*/
typedef enum gbs_ubench_output_mode_e {
	UBENCH_OUTMODE_NONE          	= 0,
	UBENCH_OUTMODE_BYTES 		 	= 1,
	UBENCH_OUTMODE_REPETITIONS 	  	= 2,
	UBENCH_OUTMODE_TIME_AVG 	 	= 3,
	UBENCH_OUTMODE_TIME_MIN 		= 4,
	UBENCH_OUTMODE_TIME_MAX 		= 5,
	UBENCH_OUTMODE_OPS_PER_SEC 		= 6,
	UBENCH_OUTMODE_BW 				= 7
} gbs_ubench_output_mode_t;

/**
* Execution mode the the multiplex function.
* When UBENCH_EXMODE_INIT is supplied the multiplex
* function should provide initial information about the
* benchmark and possibly allocate required data structures.
* When UBENCH_EXMODE_BENCH is supplied the multiplex 
* function should execute the benchmark.
* When UBENCH_EXMODE_FINISH is supplied the multiplex
* function should deallocate resources and clean up.
*/
typedef enum gbs_ubench_execution_mode_e {
	UBENCH_EXMODE_INIT,
	UBENCH_EXMODE_BENCH,
	UBENCH_EXMODE_FINISH
} gbs_ubench_execution_mode_t;

/**
* Structure to report the results of 
* a benchmark run.
* With 'valuesInvalid' it is indicated that
* these benchmark results are invalid and should be
* ignored when reporting the results.
*/
typedef struct gbs_ubench_result_s {
	double measuredTimeAvg;
	double measuredTimeMin;
	double measuredTimeMax;
	double opsPerSec;
	double measuredBandwidth;
	boolean valuesInvalid;
} gbs_ubench_result_t;

/**
* Structure that reports information about
* a microbenchmark. 
* minSize is the minimum transfer size that this 
*	benchmark supports.
* maxSize is the maximum transfer size that this 
*	benchmark supports.
* minSegmentSize is the minimum segment size that
*	is required by this benchmark (if this does not
*	correspond to the transfer sizes).
* noByteDependence signals that this benchmark does not
*	depend on a transfer size.
* outputMode The bit set of the metrics reported in the
*	benchmarks result.
*/
typedef struct gbs_ubench_bench_info_s {
	unsigned int minSize;
	unsigned int maxSize;
	unsigned int minSegmentSize;
	boolean noByteDependence;
	gbs_ubench_output_mode_t outputMode;
} gbs_ubench_bench_info_t;

/**
* Structure that holds all information for a microbenchmark.
* customData Data that may be passed from the registered benchmark
*	function to the multiplex function.
* measureIterations Number of measurement repetitions.
* warmupIterations Number of warmup repetitions before the measurement.
* dataSize The current data size for the transfers.
* aggregateCount Number of transfers to aggregate (if the benchmark supports this).
* iproc Identifier of the current GASPI Process/Rank.
* nproc Number of all GASPI ranks.
* sourceSid SID that is used as source for transfers.
* destSid SID that is used as destination at the remote end.
* resultsSid SID that is used to aggregate the results.
* benchResult Structure that holds the benchmark's results.
* timeCombined Flag if the results of all ranks shall be combined.
*/
typedef struct gbs_ubench_data_s {
	void *customData;
	unsigned int measureIterations;
	unsigned int warmupIterations;
	unsigned int dataSize;
	unsigned int aggregateCount;
	gaspi_rank_t iproc;
	gaspi_rank_t nproc;
	gaspi_segment_id_t sourceSid;
	gaspi_segment_id_t destSid;
	gaspi_segment_id_t resultsSid;
	gbs_ubench_result_t benchResult;
	boolean timeCombined;
} gbs_ubench_data_t;

/**
* Function pointer for the multiplex function that switches between benchmark management and execution
* for the individual benchmarks.
*/
typedef void (*gbs_microbenchmark_func)(gbs_ubench_data_t *, gbs_ubench_bench_info_t *, gbs_ubench_execution_mode_t);

/**
* Function to launch a microbenchmark via the microbenchmark executor.
* This function executes the benchmark for different transfer sizes, if necessary,
* and allocates the GASPI segments. Furthermore, the benchmark results are printed.
*
* @param conf The benchmark configuration with the command-line arguments.
* @param benchFunc Pointer to the multiplex function for this benchmark.
* @param customData Pointer to custom data for the benchmark. Is not used but only stored 
*	 	 to the gbs_ubench_data_t.
*/
void gbs_microbenchmark_execute_custom(gbs_bench_config_t *conf, gbs_microbenchmark_func benchFunc, void *customData);

/**
* Same as gbs_microbenchmark_execute_custom but without custom data.
*
* @param conf The benchmark configuration with the command-line arguments.
* @param benchFunc Pointer to the multiplex function for this benchmark.
*/
void gbs_microbenchmark_execute(gbs_bench_config_t *conf, gbs_microbenchmark_func benchFunc);

/**
* Intermediate function to add a measured result to the benchmark data.
* This function should be used whenever a new result has been measured for
* a benchmark. It aggregates the results of all repetitions.
*
* @param result Pointer to the result structure.
* @param usecs Number of microseconds measured for the benchmark execution.
*/
void gbs_microbenchmark_result_add(gbs_ubench_result_t *result, double usecs);

/**
* Finalize the gbs_ubench_result_t structure in the benchmark data. This function
* should be called after all repetitions of one benchmark are performed.
*
* @param benchData The benchmark data.
* @param bandwidthMultiplier Factor that the calculated bandwidth is multiplied with. The
*		 bandwidth is regarded as a unidirectional metric -- bidirectional transfers get
*		 a multiplier of '1'. If multiple transfers happen serially and the aggregated time
*		 is measured, the number of transfers is the factor.
*/
void gbs_microbenchmark_result_finish(gbs_ubench_data_t *benchData, unsigned int bandwidthMultiplier);


#endif
