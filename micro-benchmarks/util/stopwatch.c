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


stopwatch_t stopwatch_start() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return stopwatch_secsToNsecs(time.tv_sec) + time.tv_nsec;
}

stopwatch_t stopwatch_stop(stopwatch_t startTime) {
    return stopwatch_start() - startTime;
}

double stopwatch_getUsecs(stopwatch_t time) {
	return (time / 1000.0);
}
