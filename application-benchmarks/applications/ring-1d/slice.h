#ifndef SLICE_H
#define SLICE_H

#include "constant.h"

#include <GASPI.h>
#include <omp.h>

#include <stdlib.h>

typedef struct slice_t
{
  omp_lock_t lock;
  volatile int stage;
  int index;
  struct slice_t *left;
  struct slice_t *next;
} slice;

void init_slices (slice *ssl);

void handle_slice ( slice *sl, double*
		    , unsigned short* left_data_available
		    , unsigned short* right_data_available
		    , unsigned char
		    , unsigned char
		    , int num
			, int VLEN
		    );

slice* get_slice_and_lock (slice* const ssl, const int num);

#endif
