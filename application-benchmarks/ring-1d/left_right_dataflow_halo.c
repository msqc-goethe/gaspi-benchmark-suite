/*
 * This file is part of a small series of tutorial,
 * which aims to demonstrate key features of the GASPI
 * standard by means of small but expandable examples.
 * Conceptually the tutorial follows a MPI course
 * developed by EPCC and HLRS.
 *
 * Contact point for the MPI tutorial:
 *                 rabenseifner@hlrs.de
 * Contact point for the GASPI tutorial:
 *                 daniel.gruenewald@itwm.fraunhofer.de
 *                 mirko.rahn@itwm.fraunhofer.de
 *                 christian.simmendinger@t-systems.com
 */

#include "assert.h"

#ifdef USE_GASPI
#include "success_or_die.h"
#include <GASPI.h>
#endif
#include "constant.h"
#include "data.h"
#include "topology.h"
#include "slice.h"
#include "comm_util.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int main (int argc, char *argv[])
{
  
  struct timeval t1, t2;
  double elapsedTime;
  int VLEN = (64*1024);  
  double *array;
  if (argv[1]!=NULL){
    sscanf( argv[1], "%d", &VLEN);
  }

  INIT(sizeof(int)*255+NWAY * (NTHREADS + 2) * 4 * VLEN * sizeof (double));
  BARRIER_ALL();
  VLEN = VLEN / 4;
  malloc_all(NWAY * (NTHREADS + 2) * 2 * VLEN * sizeof(double), (void **)&array);
  malloc_all(sizeof(int)*255, (void **)&notify);
  for (int i = 0; i < 255;i++)
  {
    *(notify+i) = 255;
  }

  // initial buffer id
  int buffer_id = 0;

  // set notification values
  unsigned short left_data_available[NWAY];
  unsigned short right_data_available[NWAY];
  for (unsigned short id = 0; id < NWAY; ++id)
  {
    left_data_available[id] = id;
    right_data_available[id] = NWAY + id;
  }

  // initialize slice data structures
  slice *ssl = (slice *) malloc (NTHREADS * sizeof (slice));
  ASSERT (ssl);
  
  init_slices (ssl);

  // initialize data
  data_init (iProc, buffer_id, array, VLEN);


  const int right_halo  = NTHREADS+1;
  
  const int left_halo   = 0;
  BARRIER_ALL();
  gettimeofday(&t1, NULL);
  PUTMEM(array + array_OFFSET_lower(buffer_id, right_halo, 0),
          array + array_OFFSET_lower(buffer_id, left_halo + 1, 0),
          VLEN, LEFT(iProc, nProc));
  
  SEND_NOTIFY(LEFT(iProc, nProc), right_data_available[buffer_id], 1);
  // issue initial write to right ngb
  PUTMEM(array + array_OFFSET_upper(buffer_id, left_halo, 0),
          array + array_OFFSET_upper(buffer_id, right_halo - 1, 0),
          VLEN, RIGHT(iProc, nProc));
  SEND_NOTIFY(RIGHT(iProc, nProc), left_data_available[buffer_id], 1);
  const int num = NITER;
  int count= 0;
  omp_set_num_threads(NTHREADS);

#ifdef USE_MPI3
#pragma omp parallel default(none) firstprivate(buffer_id, queue_id, VLEN) \
    shared(array, left_data_available, right_data_available, ssl, stderr, count, iProc, window)
#else
#pragma omp parallel default(none) firstprivate(buffer_id, queue_id, VLEN) \
    shared(array, left_data_available, right_data_available, ssl, stderr, count, iProc)
#endif
  {
    slice* sl;
    while ((sl = get_slice_and_lock(ssl, num)))
    {
      //printf("Test %d rank: %d\n", count, iProc);
      if (omp_get_thread_num() == 0)
        count = count + 1;
      handle_slice(sl, array, left_data_available, right_data_available, segment_id, queue_id, num, VLEN);
      omp_unset_lock (&sl->lock);
    }
#pragma omp barrier
  }
  if (iProc == 0)printf("count %d \n", count);
  BARRIER_ALL();
  gettimeofday(&t2, NULL);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  if (iProc == 0)
    printf("#  nProc %d vlen %i niter %d nthreads %i nway %i time %.3f ms\n", nProc, VLEN, NITER, NTHREADS, NWAY, elapsedTime);
  BARRIER_ALL();
  FINALIZE();

  return 0;
}
