#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__
#include <inttypes.h>
#include <stdio.h>

typedef double stopwatch_t;
typedef stopwatch_t (*Timer)(void);
extern Timer benchmark_timer;

void init_timer(Timer *t);
stopwatch_t stopwatch_start();
stopwatch_t stopwatch_stop(stopwatch_t startTime);
#endif
