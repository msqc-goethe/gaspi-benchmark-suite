
#ifndef __UTIL_H
#define __UTIL_H
#include <stdio.h>
#include <unistd.h>

#ifdef USE_MPI3
#include <mpi.h>
MPI_Comm world;
MPI_Win window;
void *window_base;
size_t window_size;
void *next_window_address;
MPI_Request request;
#else
#ifdef USE_GASPI
#include <GASPI.h>
#include "success_or_die.h"
void *segment_base_ary; //first Element of GASPI Segment
void *segment_next_ary;
size_t segment_size;
#define segment_id 0
#define queue 0
#define timeout GASPI_BLOCK
#else
#include <shmem.h>
#endif
#endif


#ifdef USE_MPI3
#define QUIET(rank) MPI_Win_flush (rank, window)
#else
#ifdef USE_GASPI
#define QUIET(rank) gaspi_wait(queue, timeout);
#else
#define QUIET(rank)		shmem_quiet()
#endif
#endif


#ifdef USE_MPI3
#define LONG_LONG_GET(target, source, rank)	MPI_Get(target, 1, MPI_LONG, rank, (void *)source - window_base, 1, MPI_LONG, window); 
#else
#ifdef USE_GASPI
#define LONG_LONG_GET(target, source, rank)	gaspi_read(segment_id, (void *)target - (void *)segment_base_ary, rank, segment_id, (void *)source - (void *)segment_base_ary, 1 * sizeof(long long),queue, timeout); 
#else
#define LONG_LONG_GET(target, source, pe)		shmem_longlong_get((long long*)target, (long long*)source, 1, pe)
#endif
#endif


#ifdef USE_MPI3
#define LONG_LONG_PUT(target, source, rank)	MPI_Put(source, 1, MPI_SHORT, rank, (void *)target - window_base, 1, MPI_LONG, window); 
#else
#ifdef USE_GASPI
#define LONG_LONG_PUT(target, source,rank)	gaspi_write(segment_id, (void*)source -segment_base_ary, rank, segment_id, (void*)target -segment_base_ary, 1 * sizeof(long long), queue, timeout); 
#else
#define LONG_LONG_PUT(target, source, pe)	shmem_longlong_put(target, source, 1, pe)
#endif
#endif

#ifdef USE_MPI3
#define BARRIER_ALL()	MPI_Win_flush_all(window); MPI_Barrier(MPI_COMM_WORLD)
#else
#ifdef USE_GASPI
#define BARRIER_ALL() QUIET(); gaspi_barrier(GASPI_GROUP_ALL, timeout)
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
  *address = shmem_malloc(size);
  if (*address == NULL)
    return -1;
  else
    return 0;
}
#endif
#endif


#ifdef USE_MPI3
#define FREE_ALL(address) /* unable to free memory like this */
#else
#ifdef USE_GASPI
#define FREE_ALL(address)/* unable to free memory like this */
#else
#define FREE_ALL(address) shmem_free(address)
#endif
#endif

#ifdef USE_MPI3
#define FINALIZE() MPI_Finalize()
#else
#ifdef USE_GASPI 
#define FINALIZE() gaspi_proc_term(GASPI_BLOCK)
#else
#define FINALIZE() shmem_finalize()
#endif
#endif
#endif

