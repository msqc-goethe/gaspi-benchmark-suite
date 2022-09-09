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
 ________________________________________________________________________
  This file is adapted by Diana Waschbuesch, Heidelberg University. 
*/

#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>

#ifdef USE_MPI3
#include <mpi.h>
extern MPI_Comm world;
extern MPI_Win window;
extern void *window_base;
extern size_t window_size;
extern void *next_window_address;
extern MPI_Request request;
#else
#ifdef SGI_SHMEM
#include <mpp/shmem.h>
#else
#ifdef USE_GASPI
#include <GASPI.h>
#else
#include <shmem.h>
#endif
#endif
#endif

#ifdef USE_MPI3
#define SHORT_GET(target, source, num_elems, rank)	MPI_Get(target, num_elems, MPI_SHORT, rank, (void *)source - window_base, num_elems, MPI_SHORT, window); QUIET()
#else
#define SHORT_GET(target, source, num_elems, pe)	shmem_short_get(target, source, num_elems, pe)
#endif

#ifdef USE_MPI3
#define SHORT_GET_NB(target, source, num_elems, rank)	MPI_Get(target, num_elems, MPI_SHORT, rank, (void *)source - window_base, num_elems, MPI_SHORT, window)
#else
#define SHORT_GET_NB(target, source, num_elems, pe)	shmem_short_get_nbi(target, source, num_elems, pe)
#endif

#ifdef USE_MPI3
#define LONG_GET(target, source, num_elems, rank)	MPI_Get(target, num_elems, MPI_LONG, rank, (void *)source - window_base, num_elems, MPI_LONG, window); QUIET()
#else
#define LONG_GET(target, source, num_elems, pe)		shmem_long_get((long*)target, (long*)source, num_elems, pe)
#endif

#ifdef USE_MPI3
#define GETMEM(target, source, length, rank)		MPI_Get(target, length, MPI_BYTE, rank, (void *)source - window_base, length, MPI_BYTE, window); QUIET()
#else
#define GETMEM(target, source, length, pe)		shmem_getmem(target, source, length, pe)
#endif

#ifdef USE_MPI3
#define SHORT_PUT(target, source, num_elems, rank)	MPI_Put(source, num_elems, MPI_SHORT, rank, (void *)target - window_base, num_elems, MPI_SHORT, window); QUIET()
#else
#define SHORT_PUT(target, source, num_elems, pe)	shmem_short_put(target, source, num_elems, pe)
#endif

#ifdef USE_MPI3
#define QUIET()		MPI_Win_flush_all(window)
#else
#define QUIET()		shmem_quiet()
#endif

#ifdef USE_MPI3
#define BARRIER_ALL()	QUIET(); MPI_Barrier(MPI_COMM_WORLD)
#else
#define BARRIER_ALL()	shmem_barrier_all()
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
static inline int malloc_all(size_t size, void **address) {
  *address = shmalloc(size);
  if (*address == NULL)
    return -1;
  else
    return 0;
}
#endif

#ifdef USE_MPI3
#define FREE_ALL(address) /* unable to free memory like this */
#else
#define FREE_ALL(address) shfree(address)
#endif


#endif
