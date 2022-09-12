/**
* @file gbs_abench_grid.c
* @brief 2D 5-point stencil Halo-Exchange application benchmark for GASPI.
* @detail This benchmark computes a 5-point stencil with halo-exchange
*		  using GASPI. The heat distribution on a 2D plane is evaluted.
*		  The computation is split over multiple nodes with GASPI and
*		  additionally multithreaded using pthreads.
* @author MSQC
* @version 2020-11-24
*/

#include "gbs_abench_grid.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <utility.h>
#include <stopwatch.h>
#include <arraylist.h>
#include "success_or_die.h"
#include "gbs_benchmark_utils.h"

/*
* Square a number
*/
#define sqr(x) ((x) * (x))

/**
* Provides the number of usable rows in the FULL GRID
* (not on one shard) based on one shard's data.
*
* @param shard Pointer to a gridShard_t.
* @return Number of usable rows in the FULL grid.
*/
#define gridUsableRowCount(shard) (((shard)->cols) - 2)


/**
* Macro for easy mutex-locking. Terminates the program in case of
* an error.
*
* @param mutex Pointer to a pthread_mutex_t.
*/
#define mutexLock(mutex) {if (pthread_mutex_lock(mutex) != 0) { \
            perror("Error while locking mutex."); \
            exit(1); \
        }}


/**
* Macro for easy mutex-unlocking. Terminates the program in case of
* an error.
*
* @param mutex Pointer to a pthread_mutex_t.
*/
#define mutexUnlock(mutex) {if(pthread_mutex_unlock(mutex) != 0) { \
            perror("Error while unlocking mutex."); \
            exit(1); \
        }}


/**
* Calculate the space in bytes for a given initialized shard.
*
* @param shardPtr Pointer to a gridShard_t.
* @return Number of bytes required by the shard object and the linked area for savin the data points.
*/
#define getShardByteSize(shardPtr) (sizeof(*(shardPtr)) + (sizeof(*((shardPtr)->data)) * ((shardPtr)->rows * (shardPtr)->cols) ) )

/**
* Get the Offset for the data area in the memory segment for one shard.
* Memory Segment is layed out like: |<-- gridShard_t --> | <-- data points of the shard --> |.
*
* @param shardPtr Pointer to a gridShard_t.
* @return Number of Bytes (incl. padding) where the data array starts wrt. to the initial segment pointer.
*/
#define getSegmentShardDataOffset(shardPtr) ( ((char*)((shardPtr)->data)) - ((char*)((shardPtr))) )

/**
* Common data that is shared among all threads.
*/
typedef struct thread_base_data_t {
    gridShard_t *currentShard;
    gridShard_t *nextShard;
    pthread_mutex_t lock;
    pthread_cond_t newIteration;
    unsigned int threadsFinished;
    unsigned int threadCount;
    unsigned int currentIteration;
    unsigned int iterations;
    unsigned int iproc;
    unsigned int nproc;
} thread_base_data_t;

/**
* Data that is specific to one thread.
* - baseData: Pointer to the base data.
* - startRow: Row in the shard where the thread should start
*   	its computation (inclusive).
* - stopRow: Row in the shard where the thread should terminate
*		the computations (exclusive).
* - consumedUsecs: Number of microseconds that the thread required
*	    for its computations.
*/
typedef struct thread_data_s {
    thread_base_data_t *baseData;
    unsigned int startRow;
    unsigned int stopRow;
	double consumedUsecs;
} thread_data_t;

/**
* Notification IDs to be used when transfering
* data with GASPI.
*/
typedef enum update_nid_e {
	NID_PUT_TOP, /* This write operation writes the local top row to the remote bottom row. */
	NID_PUT_BOT /* This write operation writes the local bottom row to the remote top row. */
} update_nid_t;

/**
* Struct for result collection, possibly over all
* GASPI ranks.
*/
typedef struct grid_bench_result_s {
	double measuredTimeAvg;
	double measuredTimeMin;
	double measuredTimeMax;
	unsigned int gridSize;
	unsigned int iterations;
	unsigned int repetitions;
} grid_bench_result_t;

/**
*
* Calculates the number of rows for one Shard. The number of rows is
* distributed in a way that the first ranks get one extra row if the number
* of total rows is not evenly divisible by the number of worker ranks.
*
* @param rowCount Number of rows in the full grid (excl. shadow rows).
* @param iproc Number of the rank for which the number of rows shall be calculated.
* @param nproc Total number of GASPI ranks.
* @return Number of the data rows to be processed in this shard.
*/
static unsigned int getRowsPerProcess(unsigned int rowCount, unsigned int iproc, unsigned int nproc) {
    unsigned int rowSkip =  rowCount / nproc;
    unsigned int excessElements = rowCount - (rowSkip * nproc);
    return (iproc < excessElements) ? (rowSkip + 1): rowSkip;
}

/**
* Calculates the absolute position of the starting row of a shard in the
* resulting FULL grid. Rows are counted from 0 on.
*
* @param rowCount Number of rows in the full grid (excl. shadow rows).
* @param iproc Number of the current GASPI rank.
* @param nproc Total number of GASPI ranks.
* @return Absolute starting row in the full grid for the data of this shard.
*/
static unsigned int getAbsoluteStartRow(unsigned int rowCount, unsigned int iproc, unsigned int nproc) {
    unsigned int rowSkip =  rowCount / nproc;
    unsigned int excessElements = rowCount - (rowSkip * nproc);
    if (iproc < excessElements) {
        return (iproc * (rowSkip + 1));
    } else {
        return (excessElements * (rowSkip + 1)) + ((iproc - excessElements) * rowSkip);
    }
}

/**
* Convert a SID into a gridShard_t object. This function
* enforces the memory layout of the segment as:
*	1) gridShard_t
*	2) shard->data
* The given sid must already be allocated. The pointers in the
* returned shard object are initialized according to the memory
* layout.
*
* @param sid The segment identifier of the shard.
* @return The pointer to the shard object with correctly initialized
*		  data pointer inside.
*/
static gridShard_t *sidToShard(gaspi_segment_id_t sid) {
	void *vp;
	gridShard_t *shard;
	SUCCESS_OR_DIE(gaspi_segment_ptr(sid, &vp));
	/*
		Memory Layout:
		1) gridShard_t
		2) shard->data
	*/
	/* Note: Misuse as char* here to calculate the offset in bytes. */
	shard = (gridShard_t *)vp;
	shard->data = (gridData_t *) ((char*)vp + sizeof(*shard));
	return shard;
}

/**
* Allocates a segment for a shard and does NOT initialize the memory.
*
* @param usableRows Number of rows of the shard. The actual allocated number of rows
*		 is larger since two shadow-rows are required per shard.
* @param totalCols Number of columns in the shard. Is already larger by 2 than the usable
*		 number of columns in the grid.
* @return Pointer to the newly created shard object.
*/
static gridShard_t *allocShardFixedSize(unsigned int usableRows, unsigned int totalCols) {
    gridShard_t *shard;
	gaspi_segment_id_t sid;
	unsigned int totalRows = usableRows + 2;
	void *shardV;
	gbs_utils_create_segment(
		sizeof(*shard) + (totalRows * totalCols * sizeof(*(shard->data))),
		&sid,
		&shardV
	);

	shard = sidToShard(sid);
	shard->rows = totalRows;
	shard->cols = totalCols;
	shard->sid = sid;
    return shard;
}

extern gridShard_t *grid_allocShard(unsigned int gridSize, unsigned int iproc, int nproc) {
    unsigned int shardSize = getRowsPerProcess(gridSize, iproc, nproc);
    return allocShardFixedSize(shardSize, gridSize + 2);
}

extern grid_t *grid_allocGrid(unsigned int gridSize) {
    grid_t *grid;
    grid = safeMalloc(sizeof(*grid));

    grid->size = gridSize;
    grid->realSize = gridSize + 2;
    grid->data = safeMalloc(sqr(grid->realSize)* sizeof(*(grid->data)));
    return grid;
}

extern void grid_initializeShard(gridShard_t *shard, unsigned int iproc, unsigned int nproc, double radius, double value) {
    double center = ((shard->cols - 1) /2.0);
    double circleRadiusSqr = sqr(radius);
    unsigned int absoluteStartRow = getAbsoluteStartRow(gridUsableRowCount(shard), iproc, nproc);
    unsigned int row, rowOffset;
    for (row = 1, rowOffset = shard->cols; row < (shard->rows - 1); ++row, rowOffset += shard->cols) {
        unsigned int col;
        unsigned int absRow = absoluteStartRow + row;
        double rowTrans = absRow - center;
        for (col = 0; col < shard->cols; ++col) {
            double colTrans = col - center;
            double radius = sqr(rowTrans) + sqr(colTrans);
            unsigned int idx = rowOffset + col;
            if (radius <= circleRadiusSqr) {
                (shard->data)[idx] = value;
            } else {
                (shard->data)[idx] = 0.0;
            }
        }
    }
    memset(&((shard->data)[0]), 0, (shard->cols) * sizeof(*(shard->data))); /* obere Schattenzeile */
    memset(&((shard->data)[(shard->rows - 1) * (shard->cols)]), 0, (shard->cols) * sizeof(*(shard->data))); /* untere Schattenzeile */
}

/**
* Function to set all values in the shadow-region of the shard to zero.
* The inner data points are left untouched.
*
* @param shard Pointer to the shard in which the border should be set to zero.
*/
static void shardZeroPerimeter(gridShard_t *shard) {
    unsigned int row, rowOffset;
	/* Left and right columns */
    for (row = 1, rowOffset = shard->cols; row < shard->rows - 1; ++row, rowOffset += shard->cols) {
        shard->data[rowOffset] = 0.0;
        shard->data[rowOffset + shard->cols - 1] = 0.0;
    }
    memset(&((shard->data)[0]), 0, (shard->cols) * sizeof(*(shard->data))); /* upper shadow row */
    memset(&((shard->data)[(shard->rows - 1) * (shard->cols)]), 0, (shard->cols) * sizeof(*(shard->data))); /* lower shadow row */
}


/**
* Writes a full grid to the given stream.
*
* @param grid Pointer to the grid,
* @param stream Output stream.
* @param delimRow Delimiter between two rows.
* @param delimCol Delimiter between two columns.
*/
static void printGrid(grid_t *grid, FILE *stream, char delimRow, char delimCol) {
    if(stream != NULL) {
        unsigned int row, rowOffset;
        for (row = 1, rowOffset = grid->realSize; row < grid->realSize - 1 ; ++row, rowOffset += grid->realSize) {
            unsigned int col;
            for (col = 1; col < grid->realSize - 1; ++col) {
                fprintf(stream, "%8.4f", (grid->data)[rowOffset + col]);
                if (col < (grid->realSize - 2)) {
                    fputc(delimCol, stream);
                }
            }
            if (row < (grid->realSize - 2)) {
                fputc(delimRow, stream);
            }
        }
    }
    fputc('\n', stream);
}

/**
* Reads the four neighboring data values and returns the sum of those values.
* It must be guaranteed that the neighboring values exist.
*
* @param data Pointer to the value-array of the shard.
* @param colCount Number of columns in the shard.
* @param col Current columns.
* @param rowOffset Product of current row and number of columns.
* @return Sum of the four neighbors of the current cell.
*/
static double getNeighbourValue(gridData_t *data, unsigned int colCount, unsigned int col, unsigned int rowOffset) {
    return
        data[rowOffset + col + 1] +
        data[rowOffset + col - 1]  +
        data[rowOffset + col + colCount] +
        data[rowOffset + col - colCount];
}

/**
* Calculates the increment of one shard by one iteration. It is required that all
* data is valid in the given readShard. The function iterates over all shard
* points and applies the iteration formula.
*
* @param startRow Row from which the computation should start in the shard (inclusive).
* @param stopRow Row until which the computation is executed in the shard (exclusive).
* @param readShard The shard from which the current state is read.
* @param writeShard The shard to which the calcuated state is written.
*/
static void calculateGridIncrement(unsigned int startRow, unsigned int stopRow, gridShard_t *readShard, gridShard_t *writeShard) {
#define PHI (6.0/25.0)
    unsigned int rowOffset;
    unsigned int row;
    for (row = startRow, rowOffset = startRow*(readShard->cols); row < stopRow; ++row, rowOffset += readShard->cols) {
        unsigned int col;
        for (col = 1; col < (readShard->cols - 1); ++col) {
            (writeShard->data)[rowOffset + col] =
                ((readShard->data)[rowOffset + col] * (1 - 4.0*PHI) +
                 PHI * getNeighbourValue(readShard->data, readShard->cols, col, rowOffset));
        }
    }
#undef PHI
}

/**
* Function that sends the current shard data to the remote ranks. Only
* the thread that processed the top or bottom row will launch the
* data transfer. For this to succeed it is assumed that all ranks have
* identical segment identifiers for the same data (i.e. SID of the local
* read shard is equal to SID of remote read shard).
* After the put request, a notification is sent to the remote side that
* must be received.
*
* @param targetShard The shard in which the top or bottom row should be sent.
* @param threadData Thread-data from which startRow, target ranks, etc. are
*		 extracted.
*/
static void pushNeighborRankRows(gridShard_t *targetShard, thread_data_t *threadData) {
	unsigned int opsStarted = 0;
	unsigned int offset = getSegmentShardDataOffset(targetShard);
	/* This thread handles first row - therefore this thread must fetch the upper shadow row */
	if ((threadData->startRow == 1) && (threadData->baseData->iproc > 0)) { /* if this is not the only rank or the topmost rank */
		gaspi_rank_t targetRank = (threadData->baseData->iproc - 1);
		/* Other rank has equal number of cols (=gridSize + 2) but MAY have different number of rows */
		unsigned int targetRows = getRowsPerProcess(targetShard->cols - 2, targetRank, threadData->baseData->nproc) + 2;
		SUCCESS_OR_DIE(gaspi_write_notify(
			targetShard->sid,
			offset + sizeof(*(targetShard->data)) * targetShard->cols, /* L-offset: First actual data line */
			targetRank, /* Target Rank */
			targetShard->sid, /* Remote SID */
			offset + sizeof(*(targetShard->data)) * targetShard->cols * (targetRows - 1), /* R-offset. In the shadow row bottom*/
			sizeof(*(targetShard->data)) * targetShard->cols, /* Size */
			NID_PUT_TOP, /* Noti-ID. Every send needs different ID to avoid overwrites */
			targetRank + 1, /* Noti-value */
			0, /* Queue */
			GASPI_BLOCK
		));
		++opsStarted;
	}
	if ((threadData->stopRow == (targetShard->rows - 1)) && (threadData->baseData->iproc < (threadData->baseData->nproc - 1))) {
		gaspi_rank_t targetRank = (threadData->baseData->iproc + 1);
		SUCCESS_OR_DIE(gaspi_write_notify(
			targetShard->sid,
			offset + sizeof(*(targetShard->data)) * targetShard->cols * (targetShard->rows - 2), /* L-offset: Last data line */
			targetRank, /* Target Rank */
			targetShard->sid, /* Remote SID */
			offset + 0, /* R-offset. Top shadow row */
			sizeof(*(targetShard->data)) * targetShard->cols, /* Size */
			NID_PUT_BOT, /* Noti-ID. Every send needs different ID to avoid overwrites */
			targetRank + 1, /* Noti-value */
			0, /* Queue */
			GASPI_BLOCK
		));
		++opsStarted;
	}
	if (opsStarted > 0) {
		SUCCESS_OR_DIE(gaspi_wait( /* Flush the Queue */
			0, /* Queue */
			GASPI_BLOCK
		));
	}
}

/**
* Wait for the reception of updates from the neighboring ranks. All ranks execept
* for the first rank get their top row updated; all ranks except for the last rank
* get their bottom row updated.
* Only the threads that are responsible to process the respective updated row
* do anything in this function - the rest returns immediately.
*
* @param targetShard The shard in which rows are updated.
* @param threadData Thread-data from which startRow, target ranks, etc. are
*		 extracted.
*/
static void waitNeighborUpdates(gridShard_t *targetShard, thread_data_t *threadData) {
	/* All ranks except the last get a TOP update (meaning, some other rank sent their top row). */
	/* The other rank put its data in our last row, so the thread computing the last row should wait. */
	if ((threadData->stopRow == (targetShard->rows - 1)) && (threadData->baseData->iproc < (threadData->baseData->nproc - 1))) {
		gaspi_notification_id_t nid;
		gaspi_notification_t noti = 0;
		SUCCESS_OR_DIE(gaspi_notify_waitsome(
			targetShard->sid, /* SID */
			NID_PUT_TOP, /* Start NID */
			1, /* Number of notis */
			&nid, /* The received notification-ID */
			GASPI_BLOCK
		));
		SUCCESS_OR_DIE(gaspi_notify_reset(
			targetShard->sid, /* SID */
			nid, /* NID to reset */
			&noti /* Old noti value */
		));
		assert(noti == (threadData->baseData->iproc + 1));
	}
	/* All ranks except the first get a BOT update (i.e. some other rank sent its bottom row to us) */
	if ((threadData->startRow == 1) && (threadData->baseData->iproc > 0)) {
		gaspi_notification_id_t nid;
		gaspi_notification_t noti = 0;
		SUCCESS_OR_DIE(gaspi_notify_waitsome(
			targetShard->sid, /* SID */
			NID_PUT_BOT, /* Start NID */
			1, /* Number of notis */
			&nid, /* The received notification-ID */
			GASPI_BLOCK
		));
		SUCCESS_OR_DIE(gaspi_notify_reset(
			targetShard->sid, /* SID */
			nid, /* NID to reset */
			&noti /* Old noti value */
		));
		assert(noti == (threadData->baseData->iproc + 1));
	}
}

/**
* Main function for all PThreads of a rank. The specified number of
* iterations on the grid are performed. All threads and ranks are
* synchronized after each iteration.
*
* @param attr Pointer to an object of thread_data_t.
* @return NULL.
*/
static void *thread_computeShard(void *attr) {
    thread_data_t *data = (thread_data_t*) attr;
    thread_base_data_t *baseData = data->baseData;
    unsigned int iteration;
	stopwatch_t time;
	time = stopwatch_start();
	pushNeighborRankRows(baseData->currentShard, data);
    for (iteration = 0; iteration < baseData->iterations; ++iteration) {
		/* Wait for the halo-updates to complete */
		waitNeighborUpdates(baseData->currentShard, data);

        calculateGridIncrement(data->startRow, data->stopRow, baseData->currentShard, baseData->nextShard);
		pushNeighborRankRows(baseData->nextShard, data); /* New data is in nextShard. We push this data to the adjacent ranks which still read from currentShard */

        mutexLock(&(baseData->lock));
        ++(baseData->threadsFinished); /* This thread is done */
        if (baseData->threadsFinished < baseData->threadCount) {
            /* All threads must wait for the last thread */
            while(baseData->currentIteration == iteration) {
                pthread_cond_wait(&(baseData->newIteration), &(baseData->lock));
            }
        } else { /* Last thread is done */
            /* Switch the buffers before next iteration. */
            gridShard_t *tmp = baseData->currentShard;
            ++(baseData->currentIteration);
            baseData->currentShard = baseData->nextShard;
            baseData->threadsFinished = 0;
            baseData->nextShard = tmp;

            /* Wake all threads*/
            pthread_cond_broadcast(&(baseData->newIteration));
			 /*Sync the other ranks */
			SUCCESS_OR_DIE(gaspi_barrier(
				GASPI_GROUP_ALL,
				GASPI_BLOCK
			));
        }
        mutexUnlock(&(baseData->lock));
    }
	data->consumedUsecs = stopwatch_getUsecs(stopwatch_stop(time));
	/* We must wait for the last data exchange, even though we do not
	   need the data. Otherwise remote memory will be unavailable. */
	waitNeighborUpdates(baseData->currentShard, data);
    return NULL;
}

/**
* Copies the data and meta-data from one shard to another.
* The pointers and SID is untouched!
*
* @param dest The destination shard.
* @param src The source shard.
*/
static void shardCopy(gridShard_t *dest, gridShard_t *src) {
	assert (dest != NULL);
	assert (src != NULL);
	if (dest != src) {
		dest->rows = src->rows;
		dest->cols = src->cols;
		memcpy(dest->data, src->data, sizeof(*(src->data)) * src->cols * src->rows);
	}
}


/**
* Function that computes the given number of iterations on the given shard. The
* computation is segmented with PThreads. This function returns after all iterations
* are concluded.
*
* @param threads Number of threads to use for computation. Actual used number of threads
*		may be lower when the shard is not large enough.
* @param iterations Number of iterations (time steps) to calculate on the shard.
* @param iproc The number of this GASPI rank.
* @param nproc The total number of all GASPI ranks.
* @param shard The Shard on which the computation should be performed.
* @return Averaged number of microseconds over all threads that was consumed
*		  for the computation.
*/
static double performThreadedComputation(unsigned int threads, unsigned int iterations, unsigned int iproc, unsigned int nproc, gridShard_t *shard) {
	pthread_t *threadObjs;
	thread_data_t *threadData;
	thread_base_data_t basicData;
	gridShard_t *tmpShard;
	unsigned int thread;
	unsigned int rowSkip;
	unsigned int excessElements;
	unsigned int row = 1;
	unsigned int dataRows = (shard->rows - 2);
	double timeAvg = 0.0;

	/* Limit number of threads */
	threads = min(dataRows, threads);
	rowSkip = dataRows / threads;
	excessElements = dataRows - (rowSkip * threads);
	tmpShard = allocShardFixedSize(dataRows, shard->cols);
	basicData.currentShard = shard;
	basicData.nextShard = tmpShard;
	basicData.iterations = iterations;
	basicData.currentIteration = 0;
	basicData.threadsFinished = 0;
	basicData.threadCount = threads;
	basicData.iproc = iproc;
	basicData.nproc = nproc;
	shardZeroPerimeter(basicData.nextShard);
	if (pthread_mutex_init(&(basicData.lock), NULL) != 0) {
		perror("Error while creating lock-mutex.");
		exit(1);
	}
	if (pthread_cond_init(&(basicData.newIteration), NULL) != 0) {
		perror("Error while creating condition.");
		exit(1);
	}

	threadObjs = safeMalloc(threads * sizeof(*threadObjs));
	threadData = safeMalloc(threads * sizeof(*threadData));

	for(thread = 0; thread < threads; ++thread) {
		int threadOK;
		threadData[thread].baseData = &basicData;
		threadData[thread].startRow = row;
		threadData[thread].consumedUsecs = 0.0;
		row += rowSkip;
		/*
			If number of rows is not evenly divisible by number of
			threads, the first threads get one single extra row.
			This is fair/balanced.
		*/
		if (excessElements > 0) {
			++row;
			--excessElements;
		}
		threadData[thread].stopRow = row;
		threadOK = pthread_create(&(threadObjs[thread]), NULL, thread_computeShard, &(threadData[thread]));
		if (threadOK != 0) {
			perror("Error while creating pthread.");
			exit(1);
		}
	}
	for (thread = 0; thread < threads; ++thread) {
		int joinState = pthread_join(threadObjs[thread], NULL);
		if (joinState != 0) {
			perror("Error while joining pthread.");
			exit(1);
		}
		timeAvg += threadData[thread].consumedUsecs;
	}
	timeAvg /= threads;
	/* Wait for other ranks to conclude before teardown */
	SUCCESS_OR_DIE(gaspi_barrier(
			GASPI_GROUP_ALL,
			GASPI_BLOCK
	));
	shardCopy(shard, basicData.currentShard);
	pthread_mutex_destroy(&(basicData.lock));
	pthread_cond_destroy(&(basicData.newIteration));
	free(threadObjs);
	free(threadData);
	grid_freeShard(tmpShard);
	return timeAvg;
}

/**
* Collect all shards from all ranks to form the full grid.
* The full grid is only constructed on rank 0 - this
* function will return NULL on all other ranks. Note that
* memory for the returned grid is allocated only on rank 0
* and must therefore only be freed there.
* The collection of results is realized by a series of RMA read
* operations on rank 0. The metadata is fetched first in order to
* determine how large the remote shard is and how much data needs to
* be fetched. In the second step the actual data is loaded.
*
* @param iproc The number of this GASPI rank.
* @param nproc The total number of all GASPI ranks.
* @param The local shard.
* @return Pointer to the aggregated full grid on rank 0. Otherwise: NULL.
*/
static grid_t *aggregateShardsRank0(unsigned int iproc, unsigned int nproc, gridShard_t *shard) {
	grid_t *result = NULL;
	gridShard_t *remoteShard;
	void *vp;
	gaspi_segment_id_t sid;
	unsigned int proc;
	unsigned int rows = 1;
	gbs_utils_create_segment(
		getShardByteSize(shard),
		&sid,
		&vp
	);
	remoteShard = (gridShard_t *)vp;
	shardCopy(sidToShard(sid), shard);

	if (iproc == 0) {
		result = grid_allocGrid(shard->cols - 2);

		for (proc = 0; proc < nproc; ++proc) {
			gaspi_notification_id_t nid;
			gaspi_notification_t noti = 0;
			/* Fetch metadata */
			if (proc > 0) {
				SUCCESS_OR_DIE(gaspi_read_notify(
					sid, /* Local SID */
					0, /* Local Offset */
					proc, /* Target Rank */
					sid, /* Remote SID */
					0, /* Remote Offset */
					sizeof(*remoteShard), /* Data size */
					0, /* Noti-ID */
					0, /* Queue */
					GASPI_BLOCK /* Timeout */
				));
				SUCCESS_OR_DIE(gaspi_wait(
					0, /* Queue */
					GASPI_BLOCK
				));
				SUCCESS_OR_DIE(gaspi_notify_waitsome(
					sid, /* SID */
					0, /* Start NID */
					1, /* Number of notis */
					&nid, /* The received notification-ID */
					GASPI_BLOCK
				));
				SUCCESS_OR_DIE(gaspi_notify_reset(
					sid, /* SID */
					nid, /* NID to reset */
					&noti /* Old noti value */
				));

				remoteShard->data = (gridData_t*) ((char*)remoteShard + sizeof(*remoteShard)); /* Fix data ptr */
				/* Fetch actual data */
				SUCCESS_OR_DIE(gaspi_read_notify(
					sid, /* Local SID */
					sizeof(*shard), /* Local Offset */
					proc, /* Target Rank */
					sid, /* Remote SID */
					sizeof(*shard), /* Remote Offset */
					sizeof(*(remoteShard->data)) * remoteShard->rows * remoteShard->cols, /* Data size */
					0, /* Noti-ID */
					0, /* Queue */
					GASPI_BLOCK /* Timeout */
				));
				SUCCESS_OR_DIE(gaspi_wait(
					0, /* Queue */
					GASPI_BLOCK
				));
				SUCCESS_OR_DIE(gaspi_notify_waitsome(
					sid, /* SID */
					0, /* Start NID */
					1, /* Number of notis */
					&nid, /* The received notification-ID */
					GASPI_BLOCK
				));
				SUCCESS_OR_DIE(gaspi_notify_reset(
					sid, /* SID */
					nid, /* NID to reset */
					&noti /* Old noti value */
				));
			}
			memcpy(
				&((result->data)[remoteShard->cols * rows]),
				remoteShard->data + remoteShard->cols, /* Skip first line */
				sizeof(*(remoteShard->data)) * remoteShard->cols * (remoteShard->rows - 2) /* Read all lines except shadow */
			);
			rows += (remoteShard->rows - 2);
		}
	}

	SUCCESS_OR_DIE(gaspi_barrier(
		GASPI_GROUP_ALL,
		GASPI_BLOCK
	));

	SUCCESS_OR_DIE(gaspi_segment_delete(
		sid
	));
	return result;
}


/**
* Function that processes the whole cycle of shard initialization,
* computation of all iterations, potentially printing the resulting
* grid and deallocation of resources.
*
* @param threads Number of threads to use for computation. Actual used number of threads
*		may be lower when the shard is not large enough.
* @param iterations Number of iterations (time steps) to calculate on the shard.
* @param iproc The number of this GASPI rank.
* @param nproc The total number of all GASPI ranks.
* @param gridSize The rows/cols of the full grid.
* @param printGrid Flag to make the grid be printed out on stdout after all
*		 iterations are computed.
* @return Averaged number of microseconds over all threads per iteration that was consumed
*		  for the computation.
*/
static double grid_runShardComputation(unsigned int threads, unsigned int iterations, unsigned int iproc, unsigned int nproc, unsigned int gridSize, boolean printGrid) {
	gridShard_t *shard;
	double timeAvg;
	assert(((gridSize / nproc) > 0) && "Too many worker-processes for a too small grid!");

    shard = grid_allocShard(gridSize, iproc, nproc);
	grid_initializeShard(shard, iproc, nproc, gridSize/4.0, 127.0);
	timeAvg = performThreadedComputation(threads, iterations, iproc, nproc, shard);
	timeAvg /= iterations;
	if (printGrid) {
		grid_t *grid = aggregateShardsRank0(iproc, nproc, shard);
		if (iproc == 0) {
			grid_printGrid(grid);
		}
		grid = grid_freeGrid(grid);
	}
	shard = grid_freeShard(shard);

	return timeAvg;
}


extern void grid_printGrid(grid_t *grid) {
    printGrid(grid, stdout, '\n', '\t');
}

/**
* Writes a Shard to the given stream
*
* @param shard The Shard to be printed.
* @param stream The output stream.
* @param delimRow Delimiter between two rows.
* @param delimCol Delimiter between two columns.
*/
static void printShard(gridShard_t *shard, FILE *stream, char delimCol, char delimRow) {
    if (stream != NULL) {
        unsigned int row;
        unsigned int rowOffset;
        for (row = 0, rowOffset = 0; row < shard->rows; ++row, rowOffset += shard->cols) {
            unsigned int col;
            for (col = 0; col < shard->cols; ++col) {
                fprintf(stream, "%8.4f", (shard->data)[rowOffset + col]);
                if (col < (shard->cols - 1)) {
                    fputc(delimCol, stream);
                }
            }
            if (row < (shard->rows) - 1) {
                fputc(delimRow, stream);
            }
        }
        fputc('\n', stream);
    }
}

extern void grid_printShard(gridShard_t *shard) {
    printShard(shard, stdout, '\t', '\n');
}

extern grid_t *grid_freeGrid(grid_t *grid) {
    if(grid != NULL) {
        if(grid->data != NULL) {
            free(grid->data);
            grid->data = NULL;
        }
        free(grid);
    }
    return NULL;
}

extern gridShard_t *grid_freeShard(gridShard_t *shard) {
    if(shard != NULL) {
        if(shard->data != NULL) {
            shard->data = NULL;
        }

		SUCCESS_OR_DIE(gaspi_segment_delete(
			shard->sid
		));
    }
    return NULL;
}

/**
* Calculate the number of measurement repetitions with respect to the
* benchmark configuration from the command-line arguments.
* Depending on the grid size, less repetitions are performed.
*
* @param args The parsed command-line arguments.
* @param gridSize The size of the grid to be computed.
* @return Number of repetitions for the benchmark (w/o warm-up).
*/
static unsigned int getMeasurementIterations(cmdArgs_t *args, unsigned int gridSize) {
	unsigned int reduceThres = args->gridReduceThres;
	unsigned int factor = args->gridSizeFactor;
	unsigned int maxruns = args->gridMaxRuns;
	unsigned int minruns = args->gridMinRuns;
	unsigned int iterations;
	if ((reduceThres == 0) || (gridSize < (reduceThres))) {
		return maxruns;
	}
	reduceThres = (ceilDiv(reduceThres, factor) * factor); /* Round thres up to align with the factor */
	iterations = (maxruns / ( (gridSize * factor) / reduceThres));
	iterations = max(iterations, minruns);
	return min(iterations, maxruns);
}

/**
* Add a timing result to the result collection. The minimum and
* maximum values are tracked and the value is accumulated to the
* average. After all results are added, the average value must be
* normalized.
*
* @param result Structure to hold the result data.
* @param usecs The current timing result that should be added to
* 		 the collection.
*/
static void addResult(grid_bench_result_t *result, double usecs) {
	if (result->measuredTimeAvg == 0.0) {
		result->measuredTimeMin = usecs;
	}
	result->measuredTimeAvg += usecs;
	result->measuredTimeMax = max(result->measuredTimeMax, usecs);
}

/**
* Write the column heading line to the given stream.
* To align the results written later properly, this function
* generates a list with the columns in the stream where the
* respective column heading is written.
*
* @param f The output stream.
* @return Allocated list with integers (!) inside (misused as pointers).
*		  Numbers indicate the staring column of a column heading.
*/
static arraylist_t *printHeading(FILE *f) {
	arraylist_t *colStartList = arraylist_create();
	arraylist_t *headings = arraylist_create();
	const unsigned int separationSpaces = OUTPUT_COLUMN_SPACING;
	unsigned int i;
	unsigned int fullLength = 1;
	unsigned int currentOffset = 0;
	char *buffer = NULL;
	char spaces[separationSpaces + 1];
	arraylist_add(headings, "gridSize");
	arraylist_add(headings, "#repetitions");
	arraylist_add(headings, "#iterations");
	arraylist_add(headings, "t_min[usec]");
	arraylist_add(headings, "t_max[usec]");
	arraylist_add(headings, "t_avg[usec]");

	for (i = 0; i < arraylist_getLength(headings); ++i) {
		char *str = arraylist_get(headings, i);
		fullLength += strlen(str);
		fullLength += separationSpaces;
	}

	buffer = safeMalloc(sizeof(char) * fullLength);
	memset(spaces, ' ', separationSpaces);
	spaces[separationSpaces] = '\0';

	for (i = 0; i < arraylist_getLength(headings); ++i) {
		arraylist_add(colStartList, intToPtr(currentOffset + separationSpaces));
		currentOffset += sprintf(buffer + currentOffset, "%s%s", spaces, (char*) arraylist_get(headings, i));
	}

	fprintf(f, "%s\n", buffer);
	fflush(f);
	free(buffer);
	headings = arraylist_delete(headings);
	return colStartList;
}

/**
* Writes the benchmark results to the output stream. The numbers are separated
* by whitespace and padded so that the numbers align with the column headings.
*
* @param f The output stream.
* @param colStartList List with integers (!) in which the column start positions
*		 are encoded.
* @param result The benchmark results that shall be printed.
*/
static void printResults(FILE *f, arraylist_t *colStartList, grid_bench_result_t *result) {
	unsigned int col = 0;
	unsigned int listIdx = 0;
	unsigned int headingStart = 0;

	headingStart = ptrToInt(arraylist_get(colStartList, listIdx++));
	gbs_util_fillStreamToCol(f, &col, headingStart);
	col += fprintf(f, "%d", result->gridSize);

	headingStart = ptrToInt(arraylist_get(colStartList, listIdx++));
	gbs_util_fillStreamToCol(f, &col, headingStart);
	col += fprintf(f, "%d", result->repetitions);

	headingStart = ptrToInt(arraylist_get(colStartList, listIdx++));
	gbs_util_fillStreamToCol(f, &col, headingStart);
	col += fprintf(f, "%d", result->iterations);

	headingStart = ptrToInt(arraylist_get(colStartList, listIdx++));
	gbs_util_fillStreamToCol(f, &col, headingStart);
	col += fprintf(f, "%"DECIMAL_PRECISION, result->measuredTimeMin);

	headingStart = ptrToInt(arraylist_get(colStartList, listIdx++));
	gbs_util_fillStreamToCol(f, &col, headingStart);
	col += fprintf(f, "%"DECIMAL_PRECISION, result->measuredTimeMax);

	headingStart = ptrToInt(arraylist_get(colStartList, listIdx++));
	gbs_util_fillStreamToCol(f, &col, headingStart);
	col += fprintf(f, "%"DECIMAL_PRECISION, result->measuredTimeAvg);

	fprintf(f, "\n");
	fflush(f);
}

/**
* Potentially aggregates the benchmarl results from all ranks
* and writes the result to the given output stream.
*
* @param f The output stream.
* @param colStartList List with integers (!) in which the column start positions
*		 are encoded.
* @param result The benchmark results that shall be printed.
* @param iproc The number of this GASPI rank.
* @param nproc The total number of all GASPI ranks.
* @param timeCombined Flag if the results should be aggregated over all nodes or
*		 if only the results from the first rank should be used.
*/
static void aggregatePrintResults(FILE *f, arraylist_t *colStartList, grid_bench_result_t *result, unsigned int iproc, unsigned int nproc, boolean timeCombined) {
	if (timeCombined) {
		gaspi_segment_id_t sid;
		void *vp;
		grid_bench_result_t *resultSeg;
		gbs_utils_create_segment(
			sizeof(*result) * nproc,
			&sid,
			&vp
		);
		resultSeg = (grid_bench_result_t *) vp;
		memcpy(resultSeg, result, sizeof(*result));
		gbs_utils_gather0(sid, sizeof(*result));
		if (iproc == 0) {
			unsigned int proc;
			for (proc = 1; proc < nproc; ++proc) {
				grid_bench_result_t *procSeg = &(resultSeg[proc]);
				result->measuredTimeAvg += procSeg->measuredTimeAvg;
				result->measuredTimeMin += procSeg->measuredTimeMin;
				result->measuredTimeMax += procSeg->measuredTimeMax;
			}
			result->measuredTimeAvg /= nproc;
			result->measuredTimeMin /= nproc;
			result->measuredTimeMax /= nproc;
		}

		SUCCESS_OR_DIE(gaspi_segment_delete(
			sid
		));
	}
	if (iproc == 0) {
		printResults(f, colStartList, result);
	}
}

extern void gbs_abench_grid_stencil(gbs_bench_config_t *conf) {
	unsigned int maximumGridSize = ceilDiv(conf->cmdArgs->gridSizeEnd, conf->cmdArgs->gridSizeFactor) * conf->cmdArgs->gridSizeFactor;
	unsigned int minimumGridSize = conf->cmdArgs->gridSizeStart;
	unsigned int threads = conf->cmdArgs->gridThreads;
	unsigned int gridSize = minimumGridSize;
	arraylist_t *colStartList = NULL;
	if (conf->iproc == 0) {
		colStartList = printHeading(conf->outputStream);
	}
	while (gridSize <= maximumGridSize) {
		grid_bench_result_t result;
		unsigned int warmup;
		unsigned int repetitions;
		unsigned int i;

		memset(&result, 0, sizeof(result));
		repetitions = getMeasurementIterations(conf->cmdArgs, gridSize);
		warmup = ((conf->cmdArgs->warmupPercent) * repetitions) / 100;
		result.repetitions = repetitions;
		result.gridSize = gridSize;
		result.iterations = conf->cmdArgs->gridIterations;
		for (i = 0; i < (warmup + repetitions); ++i) {
			double timeAvg;
			timeAvg = grid_runShardComputation(threads, result.iterations, conf->iproc, conf->nproc, gridSize, false);
			if (i >= warmup) {
				addResult(&result, timeAvg);
			}
		}
		result.measuredTimeAvg /= repetitions;
		aggregatePrintResults(conf->outputStream, colStartList, &result, conf->iproc, conf->nproc, conf->cmdArgs->timeCombined);

		/* Allow for a guaranteed run with the maximum grid size */
		if ((gridSize < maximumGridSize) && ((gridSize * conf->cmdArgs->gridSizeFactor) > maximumGridSize))  {
			gridSize = maximumGridSize;
		} else {
			gridSize *= conf->cmdArgs->gridSizeFactor;
		}
	}
	colStartList = arraylist_delete(colStartList);
}
