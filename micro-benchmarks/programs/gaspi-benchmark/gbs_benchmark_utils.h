/**
* @file gbs_benchmark_utils.h
* @brief File with useful functions for the GBS benchmarks.
* @author MSQC
* @version 2020-11-25
*/
#ifndef __GBS_BENCHMARK_UTILS__
#define __GBS_BENCHMARK_UTILS__

#include <GASPI.h>
#include <stdio.h>

/**
* Decimal precision of all output metrics that are non-integer.
*/
#define DECIMAL_PRECISION ".2f"

/**
* Spacing of the columns when writing the report headers
* for a benchmark: Header entries are separated by this amount
* of spaces.
*/
#define OUTPUT_COLUMN_SPACING 4

/**
* Calculate the next GASPI queue that has at least 2 entries free.
* If the current queue is full it is tried to use the next queue
* in the order. If no sufficiently empty queue is found, the current
* queue is flushed and this function blocks.
*
* @param current Identifier of the current queue.
* @return Identifier of the next free queue that has at least
*		  2 free entries.
*/
gaspi_queue_id_t gbs_utils_getNextFreeQueue(gaspi_queue_id_t current);

/**
* Aggregate data from different ranks at rank 0. The given SID must
* correspond to a buffer at rank 0 that is sufficiently large to
* hold the values of all ranks.
*
* @param sid SID from which the data is fetched at each node and stored
*		 at rank 0.
* @param elementSize Size of one data element that should be copied.
* @return Pointer to the memory segment that is used.
*/
void *gbs_utils_gather0(gaspi_segment_id_t sid, size_t elementSize);

/**
* Write spaces to the given stream until the current column is equal to the
* target column. This is done for padding the output stream for better
* readability.
*
* @param f The stream that should be filled with spaces.
* @param currentCol Pointer to the current column. Is incremented
*		 until it reaches 'target'.
* @param target The target column until which the stream is filled.
*/
void gbs_util_fillStreamToCol(FILE * f, unsigned int *currentCol, unsigned int target);

/**
* Initialized a GASPI segment with some sample byte data.
*
* @param sid The SID of the segment to initialize.
* @param size The size of the segment.
*/
void gbs_util_initSegment(gaspi_segment_id_t sid, unsigned int size);

/**
* Generates articial CPU load by multiplying matrices.
* The load is produced for approximately the number of
* given microseconds.
*
* @param usecs Amount of microseonds to generate the artificial
*		 load for.
*/
void gbs_utils_loadCpu(double usecs);

/**
* Allocates a GASPI Segment on all nodes with the given size.
* The SID is determined automatically and is selected as the
* next free SID:
*
* @param size Size of the segment.
* @param sid Output parameter of the selected SID.
* @param vptr Output parameter for the pointer to the segment.
*/
void gbs_utils_create_segment(gaspi_size_t size, gaspi_segment_id_t *sid, void **vptr);

#endif
