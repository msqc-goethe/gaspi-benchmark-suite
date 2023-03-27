#ifndef TESTSOME_H
#define TESTSOME_H
#ifdef USE_GASPI
#include <GASPI.h>
#endif

int test_or_die ( unsigned short rank
                , unsigned char
                , unsigned short
                , unsigned int expected
                );

#endif
