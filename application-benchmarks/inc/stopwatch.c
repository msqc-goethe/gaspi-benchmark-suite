/**
* @file stopwatch.c
* @brief
* @detail 
* @author Florian Beenen
* @version 2018-10-26
*/

#include "stopwatch.h"
#include <stdlib.h>
#include <time.h>


extern stopwatch_t stopwatch_start() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return stopwatch_secsToNsecs(time.tv_sec) + time.tv_nsec;
}

extern stopwatch_t stopwatch_stop(stopwatch_t startTime) {
    return stopwatch_start() - startTime;
}

extern double stopwatch_getUsecs(stopwatch_t time) {
	return (time / 1000.0);
}

extern void stopwatch_print(stopwatch_t executionTime, FILE *descriptor, boolean withUnit) {
	if (withUnit) {
		fprintf(descriptor, "%"PRIu64 " ns", executionTime);
	} else {
		fprintf(descriptor, "%"PRIu64, executionTime);
	}
}
