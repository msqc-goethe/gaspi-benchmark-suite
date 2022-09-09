/*
   This file is part of SSCA1.

   Copyright (C) 2008-2015, UT-Battelle, LLC.

   This product includes software produced by UT-Battelle, LLC under Contract No.
   DE-AC05-00OR22725 with the Department of Energy.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the New BSD 3-clause software license (LICENSE).

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   LICENSE for more details.

   For more information please contact the SSCA1 developers at:
   bakermb@ornl.gov
*/
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#ifdef USE_MPI3
#include <mpi.h>
MPI_Comm world;
MPI_Win window;
void *window_base;
size_t window_size;
void *next_window_address;
#else
#ifdef SGI_SHMEM
#include <mpp/shmem.h>
#else
#ifdef USE_GASPI
#include <GASPI.h>
void *segment_base_ary; //first Element of GASPI Segment
void *segment_next_ary;
size_t segment_ary_size;
gaspi_notification_t noti_num;
size_t segment_size;
int seg_buf;
int seg_buf_write;
#define Queue_num 8
gaspi_queue_id_t queues[Queue_num];
#define segment_id 0
#define queue 0
#define timeout GASPI_BLOCK
#else
#include <shmem.h>
#endif
#endif
#endif



#ifdef USE_MPI3
#define QUIET() MPI_Win_flush_local_all(window)
#else
#ifdef USE_GASPI
#define QUIET() gaspi_wait(queues[omp_get_thread_num()], GASPI_BLOCK);
#else
#define QUIET()		shmem_quiet()
#endif
#endif

#ifdef USE_MPI3
#define SHORT_GET(target, source, num_elems, rank) MPI_Get(target, num_elems, MPI_SHORT, rank, (void *)source - window_base, num_elems, MPI_SHORT, window);QUIET()
#else
#ifdef USE_GASPI
#define SHORT_GET(target, source, num_elems, rank)	gaspi_read(segment_id,((void *)target - (void *)segment_base_ary),rank,segment_id,((void *)source - (void *)segment_base_ary),num_elems * sizeof(short), queues[omp_get_thread_num()],timeout);QUIET()
#else
#define SHORT_GET(target, source, num_elems, pe)	shmem_short_get(target, source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define SHORT_GET_NB(target, source, num_elems, rank)	MPI_Get(target, num_elems, MPI_SHORT, rank, (void *)source - window_base, num_elems, MPI_SHORT, window)
#else
#ifdef USE_GASPI
#define SHORT_GET_NB(target, source, num_elems, rank) gaspi_read(segment_id, ((void *)target - (void *)segment_base_ary), rank, segment_id, ((void *)source - (void *)segment_base_ary), num_elems * sizeof(uint16_t), queues[omp_get_thread_num()], timeout)
#else
#define SHORT_GET_NB(target, source, num_elems, pe)	shmem_short_get_nbi(target, source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define LONG_GET(target, source, num_elems, rank)	MPI_Get(target, num_elems, MPI_LONG, rank, (void *)source - window_base, num_elems, MPI_LONG, window);QUIET()
#else
#ifdef USE_GASPI
#define LONG_GET(target, source, num_elems, rank)	gaspi_read(segment_id, (void *)target - (void *)segment_base_ary, rank, segment_id, (void *)source - (void *)segment_base_ary, num_elems * sizeof(index_t),queues[omp_get_thread_num()], timeout); QUIET()
#else
#define LONG_GET(target, source, num_elems, pe)		shmem_long_get((long*)target, (long*)source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define GETMEM(target, source, length, rank)		MPI_Get(target, length, MPI_BYTE, rank, (void *)source - window_base, length, MPI_BYTE, window);QUIET()
#else
#ifdef USE_GASPI
#define GETMEM(target, source, length, rank)		gaspi_read(segment_id, (void *)target - segment_base_ary, rank,segment_id,(void *)source - segment_base_ary, length, queues[omp_get_thread_num()], timeout); QUIET()
#else
#define GETMEM(target, source, length, pe)		shmem_getmem(target, source, length, pe)
#endif
#endif

#ifdef USE_MPI3
#define SHORT_PUT(target, source, num_elems, rank) MPI_Put(source, num_elems, MPI_SHORT, rank, (void *)target - window_base, num_elems, MPI_SHORT, window);QUIET()
#else
#ifdef USE_GASPI
#define SHORT_PUT(target, source, num_elems, rank)	gaspi_write(segment_id, (void*)source -segment_base_ary, rank, segment_id, (void*)target -segment_base_ary, num_elems * sizeof(short), queues[omp_get_thread_num()], timeout); QUIET()
#else
#define SHORT_PUT(target, source, num_elems, pe)	shmem_short_put(target, source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define SHORT_PUT_NB(target, source, num_elems, rank) 	MPI_Put(source, num_elems, MPI_SHORT, rank, (void *)target - window_base, num_elems, MPI_SHORT, window)
#else
#ifdef USE_GASPI
#define SHORT_PUT_NB(target, source, num_elems, rank)	gaspi_write(segment_id, (void*)source -segment_base_ary, rank, segment_id, (void*)target -segment_base_ary, num_elems * sizeof(short), queues[omp_get_thread_num()], timeout)
#else
#define SHORT_PUT_NB(target, source, num_elems, pe)	SHORT_PUT(target, source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define BARRIER_ALL()	MPI_Win_flush_local_all(window); MPI_Barrier(MPI_COMM_WORLD)
#else
#ifdef USE_GASPI
#define BARRIER_ALL() QUIET(); gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK)
#else
#define BARRIER_ALL()	shmem_barrier_all()
#endif
#endif

#ifdef USE_MPI3
static inline int malloc_all(size_t size, void **address) {
  *address = next_window_address;
  next_window_address += size;
  MPI_Barrier(MPI_COMM_WORLD);
  if (next_window_address - window_base > window_size) {
    printf("ran out of memory!\n");
    return -1;
  } else
    return 0;
}
#else
#ifdef USE_GASPI

static inline int malloc_all(size_t size, void **address) {
  *address = segment_next_ary;
  segment_next_ary += size;
  BARRIER_ALL();
  if (segment_next_ary - segment_base_ary > segment_size)
  {
    printf("ran out of memory!\n");
    return -1;
  }
  else
    return 0;
}
#else
static inline int malloc_all(size_t size, void **address) {
  *address = shmalloc(size);
  if (*address == NULL)
    return -1;
  else
    return 0;
}

#endif
#endif

#ifdef USE_MPI3
#define FREE_ALL(address)
#else
#ifdef USE_GASPI
#define FREE_ALL(address)
#else
#define FREE_ALL(address) shfree(address)
#endif
#endif


#ifdef USE_MPI3
#define WAIT_NB() QUIET()
#else
#ifdef USE_GASPI
#define WAIT_NB() QUIET()
#else
#define WAIT_NB() QUIET()
#endif
#endif
