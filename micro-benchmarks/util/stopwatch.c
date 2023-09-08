#include "stopwatch.h"
#include <stdlib.h>
#include <time.h>
#include "check.h"
#include "util.h"
#include "GASPI.h"
#include "GASPI_Ext.h"

Timer benchmark_timer;
gaspi_float cpu_freq;
double cpu_div;

#define stopwatch_secsToNsecs(secs) ((secs) * (uint64_t) 1e9)
#define stopwatch_secsToNsecs(secs) ((secs) * (uint64_t) 1e9)

stopwatch_t gbs_gettime(void) {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return (stopwatch_t) (stopwatch_secsToNsecs(time.tv_sec) + time.tv_nsec);
}

// produces really weird results.
stopwatch_t gaspi_gettime(void) {
	gaspi_time_t wtime;
	GASPI_CHECK(gaspi_time_get(&wtime));
	return (stopwatch_t) (wtime * 1e6); // msecs to nsecs
}

stopwatch_t gaspi_ticks_time(void) {
	gaspi_cycles_t ticks;
	GASPI_CHECK(gaspi_time_ticks(&ticks));
	return (stopwatch_t)stopwatch_secsToNsecs(ticks * cpu_div);
}

void init_timer(Timer* t) {
	switch(options.gaspi_timer){
		case 0:
			*t = gbs_gettime;
			break;
		case 1:
			*t = gaspi_gettime;
			break;
		case 2:
			*t = gaspi_ticks_time;
			break;
		default:
			*t = gbs_gettime;
			break;
	}
	GASPI_CHECK(gaspi_cpu_frequency(&cpu_freq));
	cpu_div = 1 / cpu_freq / (1000.0 * 1000.0);
}

stopwatch_t stopwatch_start() {
	return benchmark_timer();
}

stopwatch_t stopwatch_stop(stopwatch_t startTime) {
	stopwatch_t wtime = stopwatch_start() - startTime;
	return wtime;
}
