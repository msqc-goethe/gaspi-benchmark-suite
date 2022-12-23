#include "gbs_benchmark_utils.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <utility.h>
#include <stopwatch.h>
#include "success_or_die.h"


gaspi_queue_id_t gbs_utils_getNextFreeQueue(gaspi_queue_id_t current) {
	gaspi_number_t sizeMax;
	gaspi_number_t queues;
	gaspi_queue_id_t q;
	SUCCESS_OR_DIE(gaspi_queue_size_max(
		&sizeMax
	));
	SUCCESS_OR_DIE(gaspi_queue_num(
		&queues
	));
	for(q = 0; q < queues; ++q) {
		gaspi_queue_id_t actual = (q + current) % queues;
		gaspi_number_t util;
		SUCCESS_OR_DIE(gaspi_queue_size(
			actual,
			&util
		));
		/* Need space for 2 entries: Operation+Notify*/
		if (util <= (sizeMax - 2)) {
			return actual;
		}
	}
	/* No queue was empty. Let's wait. */
	SUCCESS_OR_DIE(gaspi_wait( /* Flush the Queue */
		current, /* Queue */
		GASPI_BLOCK
	));
	return current;
}

void gbs_util_fillStreamToCol(FILE * f, unsigned int *currentCol, unsigned int target) {
	unsigned int i;
	for (i = *currentCol; i < target; ++i) {
		fputc(' ', f);
	}
	*currentCol = target;
}

void gbs_util_initSegment(gaspi_segment_id_t sid, unsigned int size) {
	void *vP;
	char *dP;
	unsigned int i;
	SUCCESS_OR_DIE(gaspi_segment_ptr(sid, &vP));
	dP = (char *) vP;
	for (i = 0; i < size; ++i) {
		dP[i] = (i + 1) % 0xFF;
	}
}

void *gbs_utils_gather0(gaspi_segment_id_t sid, size_t elementSize) {
	gaspi_rank_t iproc;
	gaspi_rank_t nproc;
	gaspi_rank_t proc;
	void *result;
	SUCCESS_OR_DIE(gaspi_proc_rank(&iproc));
	SUCCESS_OR_DIE(gaspi_proc_num(&nproc));
	SUCCESS_OR_DIE(gaspi_segment_ptr(sid, &result));
	
	if (iproc == 0) {
		gaspi_queue_id_t currentQueue = 0;
		gaspi_queue_id_t maxUsedQueue = 0;
		gaspi_notification_id_t nid;
		gaspi_notification_t noti = 0;
		for (proc = 1; proc < nproc; ++proc) {
			currentQueue = gbs_utils_getNextFreeQueue(currentQueue); 
			maxUsedQueue = max(maxUsedQueue, currentQueue);
			SUCCESS_OR_DIE(gaspi_read_notify(
				sid, /* Local SID */
				proc * elementSize, /* Local Offset */
				proc, /* Target Rank */
				sid, /* Remote SID */
				0, /* Remote Offset */
				elementSize, /* Data size */
				proc, /* Noti-ID */
				currentQueue, /* Queue */
				GASPI_BLOCK /* Timeout */
			));
		}
		for (proc = 1; proc < nproc; ++proc) {
			SUCCESS_OR_DIE(gaspi_notify_waitsome(
				sid, /* SID */
				0, /* Start NID */
				nproc, /* Number of notis */
				&nid, /* The received notification-ID */
				GASPI_BLOCK
			));
			SUCCESS_OR_DIE(gaspi_notify_reset(
				sid, /* SID */
				nid, /* NID to reset */
				&noti /* Old noti value */
			));	
		}
		/* Flush all used Queues */
		for (currentQueue = 0; currentQueue <= maxUsedQueue; ++currentQueue) {
			SUCCESS_OR_DIE(gaspi_wait( /* Flush the Queue */
				currentQueue, /* Queue */
				GASPI_BLOCK
			));
		}
	}
	SUCCESS_OR_DIE(gaspi_barrier(
		GASPI_GROUP_ALL,
		GASPI_BLOCK
	));
	return result;
}

typedef float matr_elem_t;

void gbs_utils_loadCpu(double usecs) {
	#define SIZE 16
	static matr_elem_t a[SIZE][SIZE];
	static matr_elem_t b[SIZE][SIZE];
	static volatile matr_elem_t c[SIZE][SIZE];
	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int reps = 0;
	stopwatch_t startTime;
	startTime = stopwatch_start();
	for (i = 0; i < SIZE; ++i) {
		for (j = 0; j < SIZE; ++j) {
			a[i][j] = rand();
			b[i][j] = rand();
		}
	}
	memset((matr_elem_t *)c, 0, SIZE*SIZE);
	do {
		for (i = 0; i < SIZE; ++i) {
			for (j = 0; j < SIZE; ++j) {
				for (k = 0; k < SIZE; ++k) { 
					c[i][j] += a[i][k] * b[k][j];
				}
			}
		}
		++reps;
	} while (stopwatch_getUsecs(stopwatch_stop(startTime)) < usecs);
	#undef MATR_SIZE
}

int cmpfunc (const void * a, const void * b) {
	gaspi_segment_id_t as = *((gaspi_segment_id_t *) a);
	gaspi_segment_id_t bs = *((gaspi_segment_id_t *) b);
	return as - bs;
}

void gbs_utils_create_segment(gaspi_size_t size, gaspi_segment_id_t *sid, void **vptr) {
	gaspi_number_t segCount;
	gaspi_segment_id_t *segList = NULL;
	gaspi_segment_id_t i;
	gaspi_segment_id_t newSid = 0;
	boolean slotFound = false;
	assert (sid != NULL);
	SUCCESS_OR_DIE(gaspi_segment_num(&segCount));
	segList = (gaspi_segment_id_t *)safeMalloc(segCount * sizeof(*segList));
	SUCCESS_OR_DIE(gaspi_segment_list(segCount, segList));
	qsort(segList, segCount, sizeof(gaspi_segment_id_t), cmpfunc);
	for (i = 0; (i < segCount) && !slotFound; ++i) {
		if (segList[i] == i) {
			++newSid;
		} else {
			slotFound = true;
		}
	}

	SUCCESS_OR_DIE(gaspi_segment_create(
		newSid, /* SID */
		size, /* Size */
		GASPI_GROUP_ALL, /* Group */
		GASPI_BLOCK, /* Timeout */
		GASPI_MEM_UNINITIALIZED /* Init */
	));
	*sid = newSid;
	if (vptr != NULL) {
		SUCCESS_OR_DIE(gaspi_segment_ptr(newSid, vptr));
	}
	free(segList);
}